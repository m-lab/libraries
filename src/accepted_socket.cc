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

#include "mlab/accepted_socket.h"

#if defined(OS_LINUX) || defined(OS_MACOSX) || defined(OS_ANDROID) || defined (OS_FREEBSD)
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

namespace mlab {

AcceptedSocket::AcceptedSocket(int listen_fd,
                               int accepted_fd,
                               SocketType type,
                               SocketFamily family)
    : Socket(type, family),
      listen_fd_(listen_fd),
      client_addr_len_(0) {
  fd_ = accepted_fd;
  ASSERT(type != SOCKETTYPE_TCP || listen_fd_ != -1);
}

AcceptedSocket::~AcceptedSocket() {
#if defined(OS_ANDROID) || defined(OS_LINUX) || defined(OS_MACOSX) || defined(OS_FREEBSD)
  close(listen_fd_);
#elif defined(OS_WINDOWS)
  closesocket(listen_fd_);
#else
#error Undefined platform
#endif
  listen_fd_ = -1;
}

bool AcceptedSocket::Send(const Packet& bytes, ssize_t *num_bytes) const {
  ASSERT(fd_ != -1);

  ssize_t num = -1;
  switch (type()) {
    case SOCK_STREAM:
      ASSERT(client_addr_len_ == 0);
      num = send(fd_, bytes.buffer(), bytes.length(), 0);
      break;

    case SOCK_DGRAM:
      ASSERT(client_addr_len_ != 0);
      num = sendto(fd_, bytes.buffer(), bytes.length(), 0,
                   reinterpret_cast<const sockaddr*>(&client_addr_),
                   client_addr_len_);
      break;

    default:
      LOG(FATAL, "Unexpected socket type.");
      break;
  };

  if (num_bytes != NULL)
    *num_bytes = num;
  if (num < 0) {  // errno won't change before return
    LOG(ERROR, "Failed to send: %s [%d]", strerror(errno), errno);
    return false;
  }

  if (static_cast<size_t>(num) != bytes.length()) {
    LOG(VERBOSE, "Failed to send %zu bytes; sent %zd bytes.",
        bytes.length(), num);
  }
  return true;
}

Packet AcceptedSocket::Receive(size_t count, ssize_t *num_bytes) {
  ASSERT(fd_ != -1);
  ASSERT(count > 0);

  LOG(VERBOSE, "Receiving %zu bytes.", count);

  char buffer[count];

  ssize_t num = -1;
  switch (type()) {
    case SOCK_STREAM:
      num = recv(fd_, &buffer[0], count, 0);
      break;

    case SOCK_DGRAM:
      client_addr_len_ = sizeof(sockaddr_storage);
      num = recvfrom(fd_, &buffer[0], count, 0,
                     reinterpret_cast<sockaddr*>(&client_addr_),
                     &client_addr_len_);
      break;

    default:
      LOG(FATAL, "Unexpected socket type.");
      break;
  }

  if (num_bytes != NULL)
    *num_bytes = num;
  if (num < 0) {
    LOG(ERROR, "Failed to receive: %s [%d]", strerror(errno), errno);
    return Packet("", 0);
  }

  if (static_cast<size_t>(num) != count) {
    LOG(VERBOSE, "Tried to receive %zu bytes; received %zd bytes.",
        count, num);
  }

  LOG(VERBOSE, "Received %s.", buffer);
  return Packet(buffer, num);
}

}  // namespace mlab
