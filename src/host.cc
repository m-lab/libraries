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

#include "mlab/host.h"

#if defined(OS_LINUX) || defined(OS_MACOSX) || defined(OS_ANDROID)
#include <arpa/inet.h>
#include <memory.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#elif defined(OS_WINDOWS)
#include <WinSock2.h>
#include <ws2ipdef.h>
#include <WS2tcpip.h>
#else
#error Undefined platform
#endif

#include "errno.h"
#include "log.h"

namespace mlab {

Host::Host(const std::string& hostname) : original_hostname(hostname) {
  addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = 0;
  hints.ai_protocol = 0;

  addrinfo* servinfo;
  if (int rv = getaddrinfo(hostname.c_str(), NULL, &hints, &servinfo) != 0)
    LOG(FATAL, "Failed to resolve %s: %s", hostname.c_str(), gai_strerror(rv));

  for (addrinfo* p = servinfo; p != NULL; p = p->ai_next) {
    sockaddr_storage saddr;
    std::string address;
    switch (p->ai_family) {
      case AF_INET: {
        sockaddr_in* h = reinterpret_cast<sockaddr_in*>(p->ai_addr);
        char buffer[INET_ADDRSTRLEN];
        inet_ntop(p->ai_family, &(h->sin_addr), buffer, INET_ADDRSTRLEN);
        address.assign(buffer);

        sockaddr_in* saddr_in = reinterpret_cast<sockaddr_in*>(&saddr);
        memcpy(saddr_in, h, p->ai_addrlen);
        ASSERT(saddr_in->sin_family == p->ai_family);
        break;
      }
      case AF_INET6: {
        sockaddr_in6* h = reinterpret_cast<sockaddr_in6*>(p->ai_addr);
        char buffer[INET6_ADDRSTRLEN];
        inet_ntop(p->ai_family, &(h->sin6_addr), buffer, INET6_ADDRSTRLEN);
        address.assign(buffer);

        sockaddr_in6* saddr_in6 = reinterpret_cast<sockaddr_in6*>(&saddr);
        memcpy(saddr_in6, h, p->ai_addrlen);
        ASSERT(saddr_in6->sin6_family == p->ai_family);
        ASSERT(saddr_in6->sin6_scope_id == 0);
        break;
      }
      default:
        LOG(FATAL, "Unexpected family %d", p->ai_family);
        break;
    }
    sockaddr_.push_back(saddr);
    if (resolved_ips.insert(address).second)
      LOG(VERBOSE, "Resolved: %s", address.c_str());
  }
  freeaddrinfo(servinfo);
}

SocketFamily GetSocketFamilyForAddress(const std::string& addr) {
  struct addrinfo hint;
  struct addrinfo *info;
  memset(&hint, 0, sizeof(hint));
  hint.ai_family = AF_UNSPEC;
  hint.ai_flags = AI_NUMERICHOST;  // addr is a numerical network address

  if (getaddrinfo(addr.c_str(), 0, &hint, &info) == 0) {
    switch (info->ai_family) {
      case AF_INET: return SOCKETFAMILY_IPV4;
      case AF_INET6: return SOCKETFAMILY_IPV6;
      default: return SOCKETFAMILY_UNSPEC;
    }
  } else {
    LOG(ERROR, "getaddrinfo fails. %s [%d]", strerror(errno), errno);
    return SOCKETFAMILY_UNSPEC;
  }
}

}  // namespace mlab
