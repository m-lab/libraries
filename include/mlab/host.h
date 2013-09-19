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

#ifndef _MLAB_HOST_H_
#define _MLAB_HOST_H_

#if defined(OS_LINUX) || defined(OS_MACOSX) || defined(OS_ANDROID) || defined(OS_FREEBSD)
#include <netdb.h>
#elif defined(OS_WINDOWS)
#include <WinSock2.h>
#else
#error Undefined platform.
#endif

#include <set>
#include <string>
#include <vector>

#if !defined(OS_ANDROID)
#include "gtest/gtest_prod.h"

#include "mlab/socket_family.h"

class HostTest;
#endif

namespace mlab {

typedef std::set<std::string> IPAddresses;

// A representation of a host. Used to create sockets.
class Host {
 public:
  // |hostname| can be a hostname or an IP address. Both will be checked for
  // validity.
  explicit Host(const std::string& hostname);

  // The hostname that was passed in to the constructor.
  std::string original_hostname;

  // A list of all of the IP addresses that resolve to the hostname or address
  // passed in to the constructor. These may be IPv4 or IPv6.
  IPAddresses resolved_ips;

 private:
  typedef std::vector<sockaddr_storage> SocketAddressList;

  friend class ClientSocket;
  friend class ServerSocket;
  friend class RawSocket;
#if !defined(OS_ANDROID)
  FRIEND_TEST(HostTest, ResolveLocalHostIPv4);
  FRIEND_TEST(HostTest, ResolveLocalHostIPv6);
  FRIEND_TEST(HostTest, ResolveHostname);
  FRIEND_TEST(HostTest, MultipleResolution);
#endif

  SocketAddressList sockaddr_;
};

// TODO(dominich): Move this to a better header.
SocketFamily GetSocketFamilyForAddress(const std::string& addr);
}  // namespace mlab

#endif  // _MLAB_HOST_H_
