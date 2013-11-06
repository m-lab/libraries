// Copyright 2012 M-Lab. All Rights Reserved.
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

#include "mlab/client_socket.h"

#if defined(OS_FREEBSD)
#include <netinet/in.h>
#endif

#if defined(OS_LINUX) || defined(OS_MACOSX) || defined(OS_ANDROID) || defined(OS_FREEBSD)
#include <arpa/inet.h>
#include <net/if.h>
#elif defined(OS_WINDOWS)
#include <BaseTsd.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
typedef SSIZE_T ssize_t;
#else
#error Undefined platform
#endif
#include <errno.h>
#if defined(OS_LINUX)
#include <malloc.h>
#endif
#include <string.h>

#include <vector>

#include "log.h"
#include "mlab/host.h"

namespace mlab {
// static
ClientSocket* ClientSocket::Create(const Host& hostname, uint16_t port) {
  return Create(hostname, port, SOCKETTYPE_TCP, SOCKETFAMILY_IPV4);
}

// static
ClientSocket* ClientSocket::Create(const Host& hostname,
                                   uint16_t port,
                                   SocketFamily family) {
  return Create(hostname, port, SOCKETTYPE_TCP, family);
}

// static
ClientSocket* ClientSocket::Create(const Host& hostname,
                                   uint16_t port,
                                   SocketType type) {
  return Create(hostname, port, type, SOCKETFAMILY_IPV4);
}

// static
ClientSocket* ClientSocket::Create(const Host& hostname,
                                   uint16_t port,
                                   SocketType type,
                                   SocketFamily family) {
  ClientSocket* socket = new ClientSocket(type, family);

  if (socket->fd_ == -1) {
    delete socket;
    return NULL;
  }

  if (!socket->Connect(hostname, port)) {
    socket->DestroySocket();
    LOG(FATAL, "Failed to connect to host %s|%d.",
        hostname.original_hostname.c_str(), port);
    return NULL;
  }

  return socket;
}

ClientSocket* ClientSocket::Create(const Host& bindhost, uint16_t bindport,
                                   const Host& connecthost,
                                   uint16_t connectport,
                                   SocketType type, SocketFamily family) {
  ClientSocket* socket = new ClientSocket(type, family);

  if (socket->fd_ == -1) {
    delete socket;
    return NULL;
  }

  // bind
  if (!socket->Bind(bindhost, bindport)) {
    socket->DestroySocket();
    LOG(ERROR, "Failed to bind to host %s|%d.",
        bindhost.original_hostname.c_str(), bindport);
    return NULL;
  }

  // connect
  if (!socket->Connect(connecthost, connectport)) {
    socket->DestroySocket();
    LOG(ERROR, "Failed to connect to host %s|%d.",
        connecthost.original_hostname.c_str(), connectport);
    return NULL;
  }

  return socket;
}

// static
ClientSocket* ClientSocket::CreateOrDie(const Host& hostname, uint16_t port) {
  return CreateOrDie(hostname, port, SOCKETTYPE_TCP, SOCKETFAMILY_IPV4);
}

// static
ClientSocket* ClientSocket::CreateOrDie(const Host& hostname, uint16_t port,
                                        SocketFamily family) {
  return CreateOrDie(hostname, port, SOCKETTYPE_TCP, family);
}

// static
ClientSocket* ClientSocket::CreateOrDie(const Host& hostname, uint16_t port,
                                        SocketType type) {
  return CreateOrDie(hostname, port, type, SOCKETFAMILY_IPV4);
}

// static
ClientSocket* ClientSocket::CreateOrDie(const Host& hostname, uint16_t port,
                                        SocketType type, SocketFamily family) {
  ClientSocket* socket = Create(hostname, port, type, family);
  if (!socket) {
    LOG(FATAL, "Failed to create client socket for %s:%d",
        hostname.original_hostname.c_str(), port);
  }
  return socket;
}

ClientSocket* ClientSocket::CreateOrDie(const Host& bindhost, uint16_t bindport,
                                        const Host& connecthost,
                                        uint16_t connectport,
                                        SocketType type, SocketFamily family) {
  ClientSocket* socket = Create(bindhost, bindport, connecthost, connectport,
                                type, family);

  if (!socket) {
    LOG(FATAL, "Failed to create client socket for bind %s:%d, connect %s:%d",
        bindhost.original_hostname.c_str(), bindport,
        connecthost.original_hostname.c_str(), connectport);
  }
  return socket;
}

ClientSocket::~ClientSocket() { }

bool ClientSocket::Bind(const Host& host, uint16_t port) {
  ASSERT(fd_ != 1);
  for (Host::SocketAddressList::const_iterator addr_it = host.sockaddr_.begin();
       addr_it != host.sockaddr_.end(); ++addr_it) {
    if (addr_it->ss_family != family_) {
      LOG(VERBOSE, "Socket family mismatch: Requested %d - resolved %d.",
          family_, addr_it->ss_family);
      continue;
    }

    sockaddr_storage* saddr = const_cast<sockaddr_storage*>(&(*addr_it));

    char addr_str[INET6_ADDRSTRLEN];
    socklen_t addrlen = 0;
    switch (addr_it->ss_family) {
      case AF_INET: {
        sockaddr_in* addr_in = reinterpret_cast<sockaddr_in*>(saddr);
        addr_in->sin_port = htons(port);
        addrlen = sizeof(sockaddr_in);
        inet_ntop(addr_in->sin_family, &addr_in->sin_addr, addr_str,
                  INET6_ADDRSTRLEN);
        break;
      }

      case AF_INET6: {
        sockaddr_in6* addr_in6 = reinterpret_cast<sockaddr_in6*>(saddr);
        addr_in6->sin6_port = htons(port);
        ASSERT(addr_in6->sin6_scope_id == 0);
        addrlen = sizeof(sockaddr_in6);
        inet_ntop(addr_in6->sin6_family, &addr_in6->sin6_addr, addr_str,
                  INET6_ADDRSTRLEN);
        break;
      }

      default:
        LOG(FATAL, "Unexpected IP version %u", addr_it->ss_family);
        break;
    }
    LOG(VERBOSE, "Binding to %s with family %d (AF_INET %d, AF_INET6 %d) "
                    "type %d (SOCK_STREAM %d, SOCK_DGRAM %d)",
        addr_str, family_,
        AF_INET, AF_INET6, type(), SOCK_STREAM, SOCK_DGRAM);

    if (bind(fd_, reinterpret_cast<const sockaddr*>(saddr), addrlen) == 0) {
      LOG(VERBOSE, "Bind succeed!");
      return true;
    }

    LOG(ERROR, "Failed to bind to %s on port %d: %s [%d]",
        host.original_hostname.c_str(), port, strerror(errno), errno);
  }

  LOG(ERROR, "Failed to bind to %s.", host.original_hostname.c_str());
  return false;
}

bool ClientSocket::Connect(const Host& host, uint16_t port) {
  ASSERT(fd_ != 1);

  for (Host::SocketAddressList::const_iterator addr_it = host.sockaddr_.begin();
       addr_it != host.sockaddr_.end(); ++addr_it) {
    if (addr_it->ss_family != family_) {
      LOG(VERBOSE, "Socket family mismatch: Requested %d - resolved %d.",
          family_, addr_it->ss_family);
      continue;
    }

    sockaddr_storage* saddr = const_cast<sockaddr_storage*>(&(*addr_it));

    char addr_str[INET6_ADDRSTRLEN];
    socklen_t addrlen = 0;
    switch (addr_it->ss_family) {
      case AF_INET: {
        sockaddr_in* addr_in = reinterpret_cast<sockaddr_in*>(saddr);
        addr_in->sin_port = htons(port);
        addrlen = sizeof(sockaddr_in);
        inet_ntop(addr_in->sin_family, &addr_in->sin_addr, addr_str,
                  INET6_ADDRSTRLEN);
        break;
      }

      case AF_INET6: {
        sockaddr_in6* addr_in6 = reinterpret_cast<sockaddr_in6*>(saddr);
        addr_in6->sin6_port = htons(port);
        ASSERT(addr_in6->sin6_scope_id == 0);
        addrlen = sizeof(sockaddr_in6);
        inet_ntop(addr_in6->sin6_family, &addr_in6->sin6_addr, addr_str,
                  INET6_ADDRSTRLEN);
        break;
      }

      default:
        LOG(FATAL, "Unexpected IP version %u", addr_it->ss_family);
        break;
    }
    LOG(VERBOSE, "Connecting to %s with family %d (AF_INET %d, AF_INET6 %d) "
                    "type %d (SOCK_STREAM %d, SOCK_DGRAM %d)",
        addr_str, family_,
        AF_INET, AF_INET6, type(), SOCK_STREAM, SOCK_DGRAM);

    // Connect to the first available.
    int connected;
    while ((connected = connect(fd_, reinterpret_cast<const sockaddr*>(saddr),
                                addrlen)) == -1 &&
           errno == EINTR) { }
    if (connected == 0) {
      LOG(INFO, "Connected to %s on port %d", host.original_hostname.c_str(),
          port);
      return true;
    }

    LOG(WARNING, "Failed to connect to %s on port %d: %s [%d]",
        host.original_hostname.c_str(), port, strerror(errno), errno);
  }

  LOG(ERROR, "Failed to connect to %s.", host.original_hostname.c_str());
  return false;
}

bool ClientSocket::Send(const Packet& bytes, ssize_t *num_bytes) const {
  ASSERT(fd_ != -1);

  ssize_t num;
  while ((num = send(fd_, bytes.buffer(), bytes.length(), 0)) == -1 &&
         errno == EINTR) { }
  if (num_bytes != NULL)
    *num_bytes = num;

  if (num < 0) {
    LOG(ERROR, "Failed to send: %s [%d]", strerror(errno), errno);
    return false;
  }

  if (static_cast<size_t>(num) != bytes.length()) {
    LOG(VERBOSE, "Tried to send %zu bytes; sent %zd bytes.",
        bytes.length(), num);
  }

  return true;
}

Packet ClientSocket::Receive(size_t count, ssize_t *num_bytes) const {
  ASSERT(fd_ != -1);
  ASSERT(count > 0);

  LOG(VERBOSE, "Receiving %zu bytes.", count);

  char buffer[count];
  ssize_t num;
  while ((num = recv(fd_, buffer, count, 0)) == -1 && errno == EINTR) { }
  if (num_bytes != NULL)
    *num_bytes = num;

  if (num < 0) {
    LOG(VERBOSE, "Failed to recv: %s [%d]", strerror(errno), errno);
    return Packet("", 0);
  }

  if (num == 0) {
    LOG(WARNING, "Failed to recv: No bytes available.");
  } else if (static_cast<size_t>(num) != count) {
    LOG(VERBOSE, "Tried to recv %zu bytes; recv %zd bytes instead.", count,
        num);
  }

  return Packet(buffer, num);
}

Packet ClientSocket::ReceiveX(size_t count, ssize_t *num_bytes) const {
  ASSERT(fd_ != -1);
  ASSERT(count > 0);

  LOG(VERBOSE, "Receiving %zu bytes.", count);

  char buffer[count];
  size_t offset = 0;

  while (offset < count) {
    ssize_t num;
    while ((num = recv(fd_, &buffer[offset], count - offset, 0)) == -1 &&
           errno == EINTR) { }

    if (num < 0) {
      if (num_bytes != NULL)
        *num_bytes = offset;
      LOG(VERBOSE, "Failed to recv: %s [%d]", strerror(errno), errno);
      return Packet(buffer, offset);
    }

    if (num == 0)
      break;

    offset += num;
  }
  if (num_bytes != NULL)
    *num_bytes = offset;

  if (offset == 0) {
    LOG(WARNING, "Failed to recv: No bytes available.");
  } else if (static_cast<size_t>(offset) != count) {
    LOG(VERBOSE, "Tried to recv %zu bytes; recv %zd bytes instead.", count,
        offset);
  }

  return Packet(buffer, offset);
}

ClientSocket::ClientSocket(SocketType type, SocketFamily family)
    : Socket(type, family) {
  CreateSocket();
}
}  // namespace mlab
