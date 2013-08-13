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

#include "mlab/protocol_header.h"
#include "mlab/mlab.h"
#include "log.h"

namespace mlab {

// standard checksum function, RFC1071
uint16_t InternetCheckSum(const char* buffer, int len) {
  long sum = 0;
  uint16_t rt = 0;
  uint16_t *w = (uint16_t *)buffer;
  int      nleft = len;

  while ( nleft > 1 ) {
    sum += *w++;
    nleft -= 2;
  }

  if ( nleft > 0 ) {
    sum += * (uint8_t *)w;
  }

  while ( sum >> 16 ) {
    sum = (sum & 0xffff) + (sum >> 16);
  }

  rt = ~sum;
  return (rt);
}

int IP4Header::SetSourceAddress(const std::string& addrstr) {
  return inet_pton(AF_INET, addrstr.c_str(), &source);
}

int IP4Header::SetDestinationAddress(const std::string& addrstr) {
  return inet_pton(AF_INET, addrstr.c_str(), &destination);
}

std::string IP4Header::GetSourceAddress() {
  char str[INET6_ADDRSTRLEN] = "";
  if (inet_ntop(AF_INET, &source, str, INET6_ADDRSTRLEN) == NULL)
    LOG(WARNING, "Failed to get source address");

  return std::string(str);
}

std::string IP4Header::GetDestinationAddress() {
  char str[INET6_ADDRSTRLEN] = "";
  if (inet_ntop(AF_INET, &destination, str, INET6_ADDRSTRLEN) == NULL)
    LOG(WARNING, "Failed to get source address");

  return std::string(str);
}

int IP6Header::SetSourceAddress(const std::string& addrstr) {
  return inet_pton(AF_INET6, addrstr.c_str(), &source);
}

int IP6Header::SetDestinationAddress(const std::string& addrstr) {
  return inet_pton(AF_INET6, addrstr.c_str(), &destination);
}

std::string IP6Header::GetSourceAddress() {
  char str[INET6_ADDRSTRLEN] = "";
  if (inet_ntop(AF_INET6, &source, str, INET6_ADDRSTRLEN) == NULL)
    LOG(WARNING, "Failed to get source IPv6 address");

  return std::string(str);
}

std::string IP6Header::GetDestinationAddress() {
  char str[INET6_ADDRSTRLEN] = "";
  if (inet_ntop(AF_INET6, &destination, str, INET6_ADDRSTRLEN) == NULL)
    LOG(WARNING, "Failed to get destination IPv6 address");

  return std::string(str);
}

}  // namespace mlab
