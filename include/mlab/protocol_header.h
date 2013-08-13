// Copyright 2013 M-Lab. All Rights Reserved.
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

#ifndef _MLAB_PROTOCOL_HEADER_H_
#define _MLAB_PROTOCOL_HEADER_H_

#if defined(OS_LINUX) || defined(OS_MACOSX) || defined(OS_ANDROID)
#include <arpa/inet.h>
#include <net/if.h>
#elif defined(OS_WINDOWS)
#include <WinSock2.h>
#else
#error Undefined platform
#endif

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include "mlab/packet.h"

namespace mlab {

const int mlab_default_ttl = 64;

uint16_t InternetCheckSum(const char* buffer, int len);

struct IP4Header {
  uint8_t  ver_hl;  // version header length
  uint8_t  type_of_service;
  uint16_t total_len;
  uint16_t id;
  uint16_t offset;
  uint8_t ttl;
  uint8_t protocol;
  uint16_t checksum;
  struct in_addr source, destination;

  IP4Header(uint16_t len, uint8_t ttl, uint8_t protocol, const char *dstip)
      : ver_hl((4 << 4) + 5),  // not including option head
        type_of_service(0),
        total_len(htons(len)),
        id(0),
        offset(0),
        ttl(ttl),
        protocol(protocol),
        checksum(0) {
    source.s_addr = 0;
    inet_pton(AF_INET, dstip, &destination);
  }

  int SetSourceAddress(const std::string& addrstr);

  int SetDestinationAddress(const std::string& addrstr);

  std::string GetSourceAddress();

  std::string GetDestinationAddress();
};

struct ICMP4Header {
  uint8_t icmp_type;
  uint8_t icmp_code;
  uint16_t icmp_checksum;
  uint32_t icmp_rest;

  ICMP4Header(uint8_t type, uint8_t code, uint16_t chksum, uint32_t rest)
      : icmp_type(type),
        icmp_code(code),
        icmp_checksum(chksum),
        icmp_rest(rest) {  }
};

struct IP6Header {
  uint32_t ver_tc_flow;
  uint16_t playload_len;
  uint8_t next_header;
  uint8_t hop_limit;
  struct in6_addr source;
  struct in6_addr destination;
  // seems we don't need to set it, so no constructor

  IP6Header(uint16_t len, uint8_t hl, uint8_t nh, const char *dstip)
      : ver_tc_flow(6 << 28),
        playload_len(htons(len)),
        next_header(nh),
        hop_limit(hl) {
    memset(&source, 0, sizeof(source));
    inet_pton(AF_INET6, dstip, &destination);
  }

  int SetSourceAddress(const std::string& addrstr);

  int SetDestinationAddress(const std::string& addrstr);

  std::string GetSourceAddress();

  std::string GetDestinationAddress();
};

struct ICMP6Header {
  uint8_t icmp6_type;
  uint8_t icmp6_code;
  uint16_t icmp6_checksum;
  union {
    uint32_t icmp6_data32;
    uint16_t icmp6_data16[2];
    uint8_t icmp6_data8[4];
  };

  ICMP6Header(uint8_t type, uint8_t code, uint16_t chksum, uint32_t rest)
      : icmp6_type(type),
        icmp6_code(code),
        icmp6_checksum(chksum),
        icmp6_data32(rest) {  }
};

struct UDPHeader {
  uint16_t source_port;
  uint16_t dest_port;
  uint16_t length;
  uint16_t checksum;  // check sum, could be 0, no matter.

  UDPHeader(uint16_t srcp, uint16_t dstp, uint16_t length, uint16_t chksum)
      : source_port(htons(srcp)),
        dest_port(htons(dstp)),
        length(htons(length)),
        checksum(chksum) {  }
};

}  // namespace mlab

#endif
