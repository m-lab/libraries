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

#include "mlab/socket.h"

#if defined(OS_FREEBSD)
#include <netinet/in.h>
#endif

#if defined(OS_LINUX) || defined(OS_MACOSX) || defined(OS_FREEBSD)
#include <arpa/inet.h>
#elif defined(OS_WINDOWS)
#include <winsock2.h>
#endif
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "log.h"

namespace mlab {
namespace {
int SocketProtocolFor(SocketType type, SocketFamily family) {
  switch (type) {
    case SOCKETTYPE_TCP: return 0;
    case SOCKETTYPE_UDP: return 0;
    case SOCKETTYPE_RAW:
      switch (family) {
        case SOCKETFAMILY_UNSPEC: return -1;
        case SOCKETFAMILY_IPV4: return IPPROTO_RAW;
        case SOCKETFAMILY_IPV6: return IPPROTO_ICMPV6;
      }
    case SOCKETTYPE_ICMP:
      switch (family) {
        case SOCKETFAMILY_UNSPEC: return -1;
        case SOCKETFAMILY_IPV4: return IPPROTO_ICMP;
        case SOCKETFAMILY_IPV6: return IPPROTO_ICMPV6;
      }
  }
  return -1;
}
}  // namespace

Socket::~Socket() {
  DestroySocket();
}

ssize_t Socket::SendOrDie(const Packet& bytes) const {
  ASSERT(fd_ != -1);

  ssize_t num_bytes;
  if (!Send(bytes, &num_bytes))
    LOG(FATAL, "SendOrDie failed. %s [%d]", strerror(errno), errno);
  return num_bytes;
}

Packet Socket::ReceiveOrDie(size_t count) const {
  ASSERT(fd_ != -1);
  ASSERT(count > 0);

  ssize_t num_bytes;
  Packet packet = Receive(count, &num_bytes);
  if (num_bytes < 0)
    LOG(FATAL, "ReceiveOrDie failed: %s [%d]", strerror(errno), errno);
  return packet;
}

bool Socket::SetSendBufferSize(size_t size) const {
  return SetBufferSize(size, BUFFERTYPE_SEND);
}

bool Socket::SetRecvBufferSize(size_t size) const {
  return SetBufferSize(size, BUFFERTYPE_RECV);
}

size_t Socket::GetSendBufferSize() const {
  return GetBufferSize(BUFFERTYPE_SEND);
}

size_t Socket::GetRecvBufferSize() const {
  return GetBufferSize(BUFFERTYPE_RECV);
}

Socket::Socket(SocketType type, SocketFamily family)
    : fd_(-1),
      family_(family),
      protocol_(SocketProtocolFor(type, family)),
      type_(type) {
  ASSERT(family_ != AF_UNSPEC);
  ASSERT(type_ != 0);
  ASSERT(protocol_ != -1);
}

void Socket::CreateSocket() {
  ASSERT(fd_ == -1);
  // LOG(VERBOSE, "Creating %s socket.", type_ == SOCK_STREAM ? "TCP" : "UDP");
  switch (type()) {
    case SOCK_STREAM:
      LOG(VERBOSE, "Creating TCP socket.");
      break;
    case SOCK_DGRAM:
      LOG(VERBOSE, "Creating UDP socket.");
      break;
    case SOCK_RAW:
      LOG(VERBOSE, "Creating RAW socket.");
      break;
    default:
      LOG(FATAL, "Creating UNKNOWN socket.");
      break;
  }
  fd_ = socket(family_, type(), protocol_);
  LOG(VERBOSE, "Created socket %d", fd_);
  if (fd_ == -1) {
    LOG(WARNING, "Failed to create socket %s [%d].",
        strerror(errno), errno);
  }
}

void Socket::DestroySocket() {
  if (fd_ != -1) {
#if defined(OS_ANDROID) || defined(OS_LINUX) || defined(OS_MACOSX) || defined(OS_FREEBSD)
    close(fd_);
#elif defined(OS_WINDOWS)
    closesocket(fd_);
#else
#error Undefined platform
#endif
    LOG(VERBOSE, "Destroyed socket %d", fd_);
  }
  fd_ = -1;
}

bool Socket::SetBufferSize(const size_t& size, BufferType type) const {
  ASSERT(fd_ != -1);
  ASSERT(size > 0);
  ASSERT(type == BUFFERTYPE_SEND || type == BUFFERTYPE_RECV);

  size_t bufsize = size;
  size_t bssize = sizeof(bufsize);

  int buftype = 0;

  switch (type) {
    case BUFFERTYPE_SEND: buftype = SO_SNDBUF; break;
    case BUFFERTYPE_RECV: buftype = SO_RCVBUF; break;
  }

  if (setsockopt(fd_, SOL_SOCKET, buftype, &bufsize, bssize) < 0) {
    LOG(ERROR, "Setting socket buffer fails. %s [%d]", strerror(errno), errno);
    return false;
  }

  // check real size
  size_t real_size = 0;
  getsockopt(fd_, SOL_SOCKET, buftype, &real_size,
             reinterpret_cast<socklen_t*>(&bssize));

  if (real_size < size) {  // kernel may allocate a size larger than what we set
    LOG(ERROR, "Real buffer size is smaller than we want to set. %s [%d]",
               strerror(errno), errno);
    return false;
  }

  return true;
}

size_t Socket::GetBufferSize(BufferType type) const {
  size_t bufsize = 0;
  size_t bssize = sizeof(bufsize);

  switch (type) {
    case BUFFERTYPE_SEND: {
      if (getsockopt(fd_, SOL_SOCKET, SO_SNDBUF, &bufsize,
                     reinterpret_cast<socklen_t*>(&bssize)) < 0) {
        LOG(ERROR, "get send buffer size fails. %s [%d]",
            strerror(errno), errno);
        return -1;
      }

      return bufsize;
    }
    case BUFFERTYPE_RECV: {
      if (getsockopt(fd_, SOL_SOCKET, SO_RCVBUF, &bufsize,
                     reinterpret_cast<socklen_t*>(&bssize)) < 0) {
        LOG(ERROR, "get receive buffer size fails. %s [%d]",
            strerror(errno), errno);
        return -1;
      }

      return bufsize;
    }
  }

  LOG(FATAL, "unknown buffer type.");
  return -1;
}
}  // namespace mlab
