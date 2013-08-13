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

#include <algorithm>
#include <string>
#include <vector>

#include "gtest/gtest.h"
#include "mlab/host.h"
#include "log.h"

namespace mlab {
namespace {

bool is_ipv4(const sockaddr_storage& addr) {
  return addr.ss_family == AF_INET;
}

bool is_ipv6(const sockaddr_storage& addr) {
  return addr.ss_family == AF_INET6;
}

}  // namespace

TEST(HostTest, ResolveLocalHostIPv4) {
  const char localhost_addr[] = "127.0.0.1";
  mlab::Host localhost(localhost_addr);
  EXPECT_EQ(localhost_addr, localhost.original_hostname);
  EXPECT_EQ(1U, localhost.resolved_ips.size());
  EXPECT_EQ(1U, localhost.resolved_ips.count(localhost_addr));
  EXPECT_EQ(3U, localhost.sockaddr_.size());
  // TODO(dominich): Check we get back protocols ip (0), tcp (6), udp (17)
  EXPECT_EQ(0, std::count_if(localhost.sockaddr_.begin(),
                             localhost.sockaddr_.end(),
                             is_ipv6));
  EXPECT_EQ(3, std::count_if(localhost.sockaddr_.begin(),
                             localhost.sockaddr_.end(),
                             is_ipv4));
}

TEST(HostTest, ResolveLocalHostIPv6) {
  const char localhost_addr[] = "::1";
  mlab::Host localhost(localhost_addr);
  EXPECT_EQ(localhost_addr, localhost.original_hostname);
  for (mlab::IPAddresses::const_iterator it = localhost.resolved_ips.begin();
       it != localhost.resolved_ips.end(); ++it) {
    LOG(INFO, "Resolved %s", it->c_str());
  }

  EXPECT_EQ(1U, localhost.resolved_ips.size());
  EXPECT_EQ(1U, localhost.resolved_ips.count(localhost_addr));
  EXPECT_EQ(3U, localhost.sockaddr_.size());
  // TODO(dominich): Check we get back protocols ip (0), tcp (6), udp (17)
  EXPECT_EQ(3, std::count_if(localhost.sockaddr_.begin(),
                             localhost.sockaddr_.end(),
                             is_ipv6));
  EXPECT_EQ(0, std::count_if(localhost.sockaddr_.begin(),
                             localhost.sockaddr_.end(),
                             is_ipv4));
}

TEST(HostTest, ResolveHostname) {
  const char host_addr[] = "npad.iupui.mlab1.ath01.measurement-lab.org";
  mlab::Host host(host_addr);
  EXPECT_STREQ(host_addr, host.original_hostname.c_str());
  EXPECT_EQ(2U, host.resolved_ips.size());
  EXPECT_EQ(1U, host.resolved_ips.count("83.212.4.12"));
  EXPECT_EQ(1U, host.resolved_ips.count("2001:648:2ffc:2101::12"));
  EXPECT_EQ(6U, host.sockaddr_.size());
  // TODO(dominich): Check we get back protocols ip (0), tcp (6), udp (17)
  EXPECT_EQ(3, std::count_if(host.sockaddr_.begin(),
                             host.sockaddr_.end(),
                             is_ipv6));
  EXPECT_EQ(3, std::count_if(host.sockaddr_.begin(),
                             host.sockaddr_.end(),
                             is_ipv4));
}

TEST(HostDeathTest, FailureToResolve) {
  const char host_addr[] = "hiybbprqag";
  EXPECT_DEATH(mlab::Host host(host_addr), "Failed to resolve");
}

}  // namespace mlab
