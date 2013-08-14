// Copyright 2012 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "mlab/listen_socket.h"

#if defined(OS_FREEBSD)
#include <netinet/in.h>
#endif

#if defined(OS_LINUX) || defined(OS_MACOSX) || defined(OS_ANDROID) || defined(OS_FREEBSD)
#include <arpa/inet.h>
#include <sys/socket.h>
#elif defined(OS_WINDOWS)
#include <BaseTsd.h>
#include <winsock2.h>
typedef SSIZE_T ssize_t;
#else
#error Undefined platform
#endif
#include <errno.h>
#if defined(OS_LINUX) || defined(OS_WINDOWS)
#include <malloc.h>
#endif

#include <string.h>

#include "log.h"
#include "mlab/accepted_socket.h"

namespace mlab {
namespace {
const timeval kDefaultTimeout = {5, 0};
}

// static
ListenSocket* ListenSocket::Create(uint16_t port) {
  return Create(port, SOCKETTYPE_TCP, SOCKETFAMILY_IPV4);
}

// static
ListenSocket* ListenSocket::Create(uint16_t port, SocketFamily family) {
  return Create(port, SOCKETTYPE_TCP, family);
}

// static
ListenSocket* ListenSocket::Create(uint16_t port, SocketType type) {
  return Create(port, type, SOCKETFAMILY_IPV4);
}

// static
ListenSocket* ListenSocket::Create(uint16_t port, SocketType type,
                                   SocketFamily family) {
  ListenSocket* socket = new ListenSocket(port, type, family);
  if (socket->fd_ != -1)
    return socket;

  delete socket;
  return NULL;
}

// static
ListenSocket* ListenSocket::CreateOrDie(uint16_t port) {
  return CreateOrDie(port, SOCKETTYPE_TCP, SOCKETFAMILY_IPV4);
}

// static
ListenSocket* ListenSocket::CreateOrDie(uint16_t port, SocketFamily family) {
  return CreateOrDie(port, SOCKETTYPE_TCP, family);
}

// static
ListenSocket* ListenSocket::CreateOrDie(uint16_t port, SocketType type) {
  return CreateOrDie(port, type, SOCKETFAMILY_IPV4);
}

// static
ListenSocket* ListenSocket::CreateOrDie(uint16_t port, SocketType type,
                                        SocketFamily family) {
  ListenSocket* socket = Create(port, type, family);
  if (!socket)
    LOG(FATAL, "Failed to create server socket on port %d", port);
  return socket;
}

ListenSocket::~ListenSocket() { }

void ListenSocket::Select() {
  SelectWithTimeout((uint32_t) -1);
}

void ListenSocket::SelectWithTimeout(uint32_t timeout) {
  ASSERT(fd_ != -1);

  if (type() != SOCKETTYPE_TCP)
    return;

  fd_set rfds;
  FD_ZERO(&rfds);
  FD_SET(fd_, &rfds);

  timeval timeout_val = { timeout, 0 };
  timeval* timeout_ptr = (timeout == (uint32_t) -1 ? NULL : &timeout_val);

  int connected = select(fd_ + 1, &rfds, NULL, NULL, timeout_ptr);
  if (connected == -1) {
    LOG(FATAL, "Error when attempting to select connection: %s [%d]",
        strerror(errno), errno);
  } else if (connected == 0) {
    LOG(FATAL, "Timeout when attempting to select connection: %s [%d]",
        strerror(errno), errno);
  } else {
    ASSERT(connected == 1);
    LOG(INFO, "Selected connection.");
  }
}

AcceptedSocket* ListenSocket::Accept() const {
  ASSERT(fd_ != -1);

  if (type() != SOCKETTYPE_TCP) {
    return new AcceptedSocket(fd_, -1, type(), family_);
  }

  sockaddr_storage sock_storage;
  sockaddr* saddr = reinterpret_cast<sockaddr*>(&sock_storage);
  socklen_t saddrlen = sizeof(sock_storage);
  int client_fd = accept(fd_, saddr, &saddrlen);
  if (client_fd == -1) {
    LOG(ERROR, "Failed to accept connection: %s [%d]",
        strerror(errno), errno);
    return NULL;
  }
  LOG(INFO, "Accepted connection.");
  return new AcceptedSocket(fd_, client_fd, type(), family_);
}

AcceptedSocket* ListenSocket::AcceptOrDie() const {
  AcceptedSocket* accepted = Accept();
  ASSERT(accepted != NULL);
  return accepted;
}

bool ListenSocket::Send(const Packet&, ssize_t*) const {
  ASSERT(false);
  return false;
}

Packet ListenSocket::Receive(size_t, ssize_t*) {
  ASSERT(false);
  return Packet(std::string());
}

ListenSocket::ListenSocket(uint16_t port, SocketType type, SocketFamily family)
    : Socket(type, family) {
  Start(port);
}

void ListenSocket::Start(uint16_t port) {
  CreateSocket();

  sockaddr_storage saddr;
  memset(&saddr, 0, sizeof(sockaddr_storage));
  saddr.ss_family = family_;
  socklen_t saddr_len = 0;
  char addr_str[INET6_ADDRSTRLEN];
  switch (family_) {
    case AF_INET: {
      sockaddr_in* addr_in = reinterpret_cast<sockaddr_in*>(&saddr);
      saddr_len = sizeof(sockaddr_in);
      addr_in->sin_port = htons(port);
      addr_in->sin_addr.s_addr = INADDR_ANY;
      inet_ntop(addr_in->sin_family, &addr_in->sin_addr, addr_str,
                INET6_ADDRSTRLEN);
      break;
    }

    case AF_INET6: {
      sockaddr_in6* addr_in = reinterpret_cast<sockaddr_in6*>(&saddr);
      saddr_len = sizeof(sockaddr_in6);
      addr_in->sin6_port = htons(port);
      addr_in->sin6_addr = in6addr_any;
      inet_ntop(addr_in->sin6_family, &addr_in->sin6_addr, addr_str,
                INET6_ADDRSTRLEN);
      break;
    }

    default:
      ASSERT(false);
  }

  // setsockopt on posix takes a const void* and on Windows takes a
  // const char*. Cast to the lesser of two evils.
  // Don't block forever.
  // TODO(dominich): Allow custom timeout.
  ASSERT(setsockopt(fd_, SOL_SOCKET, SO_RCVTIMEO,
                    (const char*) &kDefaultTimeout,
                    sizeof(kDefaultTimeout)) != -1);
  ASSERT(setsockopt(fd_, SOL_SOCKET, SO_SNDTIMEO,
                    (const char*) &kDefaultTimeout,
                    sizeof(kDefaultTimeout)) != -1);

  // Make sure address is reusable.
  int on = 1;
  if (setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR,
                 (const char*) &on, sizeof(on)) == -1) {
    LOG(ERROR, "Failed to set socket to reusable.");
    DestroySocket();
  }

  if (bind(fd_, reinterpret_cast<sockaddr*>(&saddr), saddr_len) == -1) {
    LOG(ERROR, "Failed to bind on port %d: %s [%d]", port,
        strerror(errno), errno);
    DestroySocket();
  } else {
    LOG(VERBOSE, "Bound to %s %d", addr_str, port);
  }

  if (type() == SOCKETTYPE_TCP) {
    if (listen(fd_, 1) == -1) {
      LOG(ERROR, "Failed to listen: %s [%d]", strerror(errno), errno);
      DestroySocket();
    } else {
      LOG(VERBOSE, "Listening on %d", port);
    }
  }

  if (fd_ == -1)
    LOG(FATAL, "Failed to find address to bind to and listen on.");
}

}  // namespace mlab
