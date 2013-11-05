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

#include "mlab/raw_socket.h"

#if defined(OS_FREEBSD)
#include <netinet/in.h>
#endif

#if defined(OS_LINUX) || defined(OS_MACOSX) || defined(OS_ANDROID) || defined(OS_FREEBSD)
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/icmp6.h>
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
RawSocket* RawSocket::Create() {
  return Create(SOCKETTYPE_RAW, SOCKETFAMILY_IPV4);
}

// static
RawSocket* RawSocket::Create(SocketType type, SocketFamily family) {
  RawSocket* socket = new RawSocket(type, family);
  if (socket->fd_ != 1)
    return socket;

  delete socket;
  return NULL;
}

// static
RawSocket* RawSocket::CreateOrDie() {
  return CreateOrDie(SOCKETTYPE_RAW, SOCKETFAMILY_IPV4);
}

// static
RawSocket* RawSocket::CreateOrDie(SocketType type, SocketFamily family) {
  RawSocket* socket = Create(type, family);
  if (!socket) {
    LOG(FATAL, "Failed to create raw socket!")
  }
  return socket;
}

RawSocket::~RawSocket() { }

int RawSocket::Connect(const Host& host) {
  ASSERT(fd_ != -1);

  for (Host::SocketAddressList::const_iterator addr_it = host.sockaddr_.begin();
       addr_it != host.sockaddr_.end(); ++addr_it) {
    if (addr_it->ss_family != family_) {
      continue;
    }

    socklen_t addrlen = 0;
    switch (addr_it->ss_family) {
      case AF_INET: addrlen = sizeof(sockaddr_in); break;
      case AF_INET6: addrlen = sizeof(sockaddr_in6); break;
    }

    if (connect(fd_, reinterpret_cast<const sockaddr*>(&(*addr_it)),
                addrlen) < 0)
      continue;
    else
      return 0;
  }

  LOG(ERROR, "Failed to connect to %s.", host.original_hostname.c_str());
  return -1;
}

int RawSocket::Bind(const Host& host) {
  ASSERT(fd_ != -1);

  for (Host::SocketAddressList::const_iterator addr_it = host.sockaddr_.begin();
       addr_it != host.sockaddr_.end(); ++addr_it) {
    if (addr_it->ss_family != family_) {
      continue;
    }

    char addr_str[INET6_ADDRSTRLEN];
    socklen_t addrlen = 0;
    // TODO(xunfan): See if the following blob can make a utility method.
    switch (addr_it->ss_family) {
      case AF_INET: {
        addrlen = sizeof(sockaddr_in);
        const sockaddr_in* addr_in =
            reinterpret_cast<const sockaddr_in*>(&(*addr_it));
        inet_ntop(addr_in->sin_family, &addr_in->sin_addr, addr_str,
                  INET6_ADDRSTRLEN);
        break;
      }
      case AF_INET6: {
        addrlen = sizeof(sockaddr_in6);
        const sockaddr_in6* addr_in =
            reinterpret_cast<const sockaddr_in6*>(&(*addr_it));
        inet_ntop(addr_in->sin6_family, &addr_in->sin6_addr, addr_str,
                  INET6_ADDRSTRLEN);
        break;
      }
    }

    if (bind(fd_, reinterpret_cast<const sockaddr*>(&(*addr_it)),
             addrlen) < 0) {
      LOG(ERROR, "cannot bind to %s %s [%d]", addr_str, strerror(errno), errno);
      continue;
    } else {
      return 0;
    }
  }

  LOG(ERROR, "Failed to bind to %s.", host.original_hostname.c_str());
  return -1;
}

bool RawSocket::Send(const Packet& bytes, ssize_t *num_bytes) const {
  ASSERT(fd_ != -1);

  const size_t packet_len = bytes.length();

  ssize_t num = send(fd_, bytes.buffer(), packet_len, 0);
  if (num_bytes != NULL)
    *num_bytes = num;

  if (num < 0) {
    LOG(ERROR, "Failed to send: %s [%d]", strerror(errno), errno);
    return false;
  }

  if (static_cast<size_t>(num) != packet_len) {
    LOG(VERBOSE, "Tried to send %zu bytes; sent %zd bytes.",
        packet_len, num);
  }

  return true;
}

void RawSocket::SendToOrDie(const Host& host, const Packet& bytes,
                            ssize_t *num_bytes) const {
  ASSERT(fd_ != -1);

  if (!SendTo(host, bytes, num_bytes))
    LOG(FATAL, "SendToOrDie fails. %s [%d]", strerror(errno), errno);
}

bool RawSocket::SendTo(const Host& host, const Packet& bytes,
                       ssize_t *num_bytes) const {
  ASSERT(fd_ != -1);
  for (Host::SocketAddressList::const_iterator addr_it = host.sockaddr_.begin();
      addr_it != host.sockaddr_.end(); ++addr_it) {
    if (addr_it->ss_family != family_) {
      continue;
    }

    socklen_t addrlen = 0;
    char addr_str[INET6_ADDRSTRLEN];

    switch (addr_it->ss_family) {
      case AF_INET: {
        const sockaddr_in *addr_in =
          reinterpret_cast<const sockaddr_in*>(&(*addr_it));
        addrlen = sizeof(sockaddr_in);
        inet_ntop(addr_in->sin_family, &addr_in->sin_addr, addr_str,
                  INET6_ADDRSTRLEN);
        break;
      }
      case AF_INET6: {
        const sockaddr_in6 *addr_in6 =
          reinterpret_cast<const sockaddr_in6*>(&(*addr_it));
        addrlen = sizeof(sockaddr_in6);
        inet_ntop(addr_in6->sin6_family, &addr_in6->sin6_addr, addr_str,
                  INET6_ADDRSTRLEN);
        break;
      }
    }

    const size_t packet_len = bytes.length();
    ssize_t num = sendto(fd_, bytes.buffer(), packet_len, 0,
                         reinterpret_cast<const sockaddr*>(&(*addr_it)), addrlen);
    *num_bytes = num;
    if (num < 0) {
      LOG(ERROR, "Failed to send to %s: %s [%d]", addr_str,
          strerror(errno), errno);
      return false;
    } else if (static_cast<size_t>(num) != packet_len) {
      LOG(ERROR, "Failed to send to %s %zu bytes; sent %zd bytes.",
          addr_str, packet_len, num);
    }

    return true;
  }

  *num_bytes = 0;
  LOG(ERROR, "no matching socketfamily address!");
  return true;
}

Packet RawSocket::Receive(size_t count, ssize_t *num_bytes) const {
  ASSERT(fd_ != -1);
  ASSERT(count > 0);

  LOG(VERBOSE, "Receiving %zu bytes.", count);

  char buffer[count];

  ssize_t num = recv(fd_, buffer, count, 0);

  if (num_bytes != NULL)
    *num_bytes = num;
  if (num < 0) {
    LOG(ERROR, "Failed to recv: %s [%d]", strerror(errno), errno);
    return Packet("");
  }

  if (num == 0) {
    LOG(WARNING, "Failed to recv: No bytes available.");
  } else if (static_cast<size_t>(num) != count) {
    LOG(VERBOSE, "Tried to recv %zu bytes; recv %zd bytes instead.", count,
        num);
  }

  return Packet(buffer, num);
}

Packet RawSocket::ReceiveFromOrDie(size_t count, Host* host) const {
  ASSERT(fd_ != -1);
  ASSERT(count > 0);

  ssize_t num_bytes;
  Packet packet = ReceiveFrom(count, host, &num_bytes);
  if (num_bytes < 0)
    LOG(FATAL, "ReceiveFromOrDie fails. %s [%d]", strerror(errno), errno);
  return packet;
}

Packet RawSocket::ReceiveFrom(size_t count,
                              Host* host,
                              ssize_t* num_bytes) const {
  ASSERT(fd_ != -1);
  ASSERT(count > 0);

  char addr_str[INET6_ADDRSTRLEN] = {0};
  char buffer[count];
  sockaddr_storage recvaddr;
  socklen_t recvaddrlen = sizeof(sockaddr_storage);
  ssize_t num = recvfrom(fd_, buffer, count, 0,
                     reinterpret_cast<sockaddr*>(&recvaddr),
                     &recvaddrlen);

  if (num_bytes != NULL)
    *num_bytes = num;
  if (num < 0) {
    LOG(ERROR, "Raw socket fails to recvfrom: %s [%d]", strerror(errno), errno);
    return Packet("", 0);
  }

  if (num == 0) {
    LOG(WARNING, "Failed to recvfrom: No bytes available.");
  } else if (static_cast<size_t>(num) != count) {
    LOG(VERBOSE, "Tried to recvfrom %zu bytes; recv %zd bytes instead.",
        count, num);
  }

  if (num > 0) {
    switch (recvaddr.ss_family) {
      case AF_INET: {
        const sockaddr_in *addr_in =
          reinterpret_cast<const sockaddr_in*>(&recvaddr);
        inet_ntop(recvaddr.ss_family, &addr_in->sin_addr, addr_str,
                  INET6_ADDRSTRLEN);
        break;
      }
      case AF_INET6: {
        const sockaddr_in6 *addr_in6 =
          reinterpret_cast<const sockaddr_in6*>(&recvaddr);
        inet_ntop(recvaddr.ss_family, &addr_in6->sin6_addr, addr_str,
                  INET6_ADDRSTRLEN);
        break;
      }
    }
    *host = Host(std::string(addr_str));
  }

  return Packet(buffer, num);
}

bool RawSocket::SetIPHDRINCL() {
  ASSERT(fd_ != -1);

  int socklevel  = 0;
  switch ( family_ ) {
    case AF_UNSPEC: return -1;
    case AF_INET: socklevel = IPPROTO_IP; break;
    case AF_INET6: socklevel = IPPROTO_IPV6; break;
  }
  int on = 1;
  return (setsockopt(fd_, socklevel, IP_HDRINCL, &on, sizeof(on)) == 0);
}

RawSocket::RawSocket(SocketType type, SocketFamily family)
    : Socket(type, family) {
  CreateSocket();
  if (fd_ == -1) {
    LOG(FATAL, "Create raw socket fails. Need root privilege, are you root?");
  }

  ASSERT(family != SOCKETFAMILY_UNSPEC);
  switch ( family ) {
    case SOCKETFAMILY_UNSPEC:  break;
    case SOCKETFAMILY_IPV4:  break;
    case SOCKETFAMILY_IPV6:
      {
        // we set ICMPv6 filters here.
        // Accepting only ECHO_REPLY, TIME_EXCEEDED, DST_UNREACH
        struct icmp6_filter myfilt;
        ICMP6_FILTER_SETBLOCKALL(&myfilt);
        ICMP6_FILTER_SETPASS(ICMP6_ECHO_REPLY, &myfilt);
        ICMP6_FILTER_SETPASS(ICMP6_TIME_EXCEEDED, &myfilt);
        ICMP6_FILTER_SETPASS(ICMP6_DST_UNREACH, &myfilt);
        if (setsockopt(fd_, IPPROTO_ICMPV6, ICMP6_FILTER,
                       &myfilt, sizeof(myfilt)) < 0) {
          LOG(FATAL, "Failed to set ICMPv6 raw socket filters! %s [%d]",
                             strerror(errno), errno);
        }
      }
      break;
  }
}

}  // namespace mlab
