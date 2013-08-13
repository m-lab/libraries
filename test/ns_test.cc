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

#include "gtest/gtest.h"
#include "log.h"
#include "mlab/mlab.h"
#include "mlab/ns.h"

namespace mlab {

extern const std::string& get_user_agent();

namespace {

const char identifier[] = "NSTest";
const char version[] = "0.1";

class NSTest : public ::testing::Test {
 protected:
  NSTest() { Initialize(identifier, version); }
};

}  // namespace

TEST_F(NSTest, UserAgent) {
  const std::string expected_user_agent = std::string("M-Lab ") + identifier +
      " (" + version + ")";
  EXPECT_EQ(0U, get_user_agent().find(expected_user_agent));
}

TEST_F(NSTest, HostForTool) {
  const std::string prefix("npad.iupui.mlab");
  const std::string suffix(".measurement-lab.org");

  mlab::Host host = mlab::ns::GetHostForTool("npad");
  LOG(INFO, "host: %s -> %s", host.original_hostname.c_str(),
      host.resolved_ips.begin()->c_str());
  EXPECT_EQ(0U, host.original_hostname.find(prefix));
  EXPECT_EQ(host.original_hostname.length() - suffix.length(),
            host.original_hostname.find(suffix));
}

TEST_F(NSTest, RandomHostForTool) {
  const std::string prefix("npad.iupui.mlab");
  const std::string suffix(".measurement-lab.org");

  mlab::Host host = mlab::ns::GetRandomHostForTool("npad");
  LOG(INFO, "host: %s -> %s", host.original_hostname.c_str(),
      host.resolved_ips.begin()->c_str());
  EXPECT_EQ(0U, host.original_hostname.find(prefix));
  EXPECT_EQ(host.original_hostname.length() - suffix.length(),
            host.original_hostname.find(suffix));
}

TEST_F(NSTest, HostForToolAndMetro) {
  const std::string prefix("npad.iupui.mlab");
  const std::string suffix(".measurement-lab.org");

  mlab::Host host = mlab::ns::GetHostForToolAndMetro("npad", "ath");
  LOG(INFO, "host: %s -> %s", host.original_hostname.c_str(),
      host.resolved_ips.begin()->c_str());
  EXPECT_EQ(0U, host.original_hostname.find(prefix));
  EXPECT_EQ(prefix.length() + 2, host.original_hostname.find("ath"));
  EXPECT_EQ(host.original_hostname.length() - suffix.length(),
            host.original_hostname.find(suffix));
}

TEST_F(NSTest, HostForToolAndFamily) {
  const std::string prefix("npad.iupui.mlab");
  const std::string suffix(".measurement-lab.org");
  // IPv4
  {
    mlab::Host host = mlab::ns::GetHostForToolAndFamily("npad",
                                                        SOCKETFAMILY_IPV4);
    LOG(INFO, "ipv4 host: %s -> %s", host.original_hostname.c_str(),
        host.resolved_ips.begin()->c_str());
    EXPECT_EQ(0U, host.original_hostname.find(prefix));
    EXPECT_EQ(prefix.length() + 1, host.original_hostname.find("v4"));
    EXPECT_EQ(host.original_hostname.length() - suffix.length(),
              host.original_hostname.find(suffix));
  }

  // IPv6
  {
    mlab::Host host = mlab::ns::GetHostForToolAndFamily("npad",
                                                        SOCKETFAMILY_IPV6);
    LOG(INFO, "ipv6 host: %s -> %s", host.original_hostname.c_str(),
        host.resolved_ips.begin()->c_str());
    EXPECT_EQ(0U, host.original_hostname.find(prefix));
    EXPECT_EQ(prefix.length() + 1, host.original_hostname.find("v6"));
    EXPECT_EQ(host.original_hostname.length() - suffix.length(),
              host.original_hostname.find(suffix));
  }
}

TEST_F(NSTest, RandomHostForToolAndFamily) {
  const std::string prefix("npad.iupui.mlab");
  const std::string suffix(".measurement-lab.org");
  // IPv4
  {
    mlab::Host host = mlab::ns::GetRandomHostForToolAndFamily(
        "npad", SOCKETFAMILY_IPV4);
    LOG(INFO, "ipv4 host: %s -> %s", host.original_hostname.c_str(),
        host.resolved_ips.begin()->c_str());
    EXPECT_EQ(0U, host.original_hostname.find(prefix));
    EXPECT_EQ(prefix.length() + 1, host.original_hostname.find("v4"));
    EXPECT_EQ(host.original_hostname.length() - suffix.length(),
              host.original_hostname.find(suffix));
  }

  // IPv6
  {
    mlab::Host host = mlab::ns::GetRandomHostForToolAndFamily(
        "npad", SOCKETFAMILY_IPV6);
    LOG(INFO, "ipv6 host: %s -> %s", host.original_hostname.c_str(),
        host.resolved_ips.begin()->c_str());
    EXPECT_EQ(0U, host.original_hostname.find(prefix));
    EXPECT_EQ(prefix.length() + 1, host.original_hostname.find("v6"));
    EXPECT_EQ(host.original_hostname.length() - suffix.length(),
              host.original_hostname.find(suffix));
  }
}

TEST_F(NSTest, HostForToolAndMetroAndFamily) {
  const std::string prefix("npad.iupui.mlab");
  const std::string suffix(".measurement-lab.org");
  // IPv4
  {
    mlab::Host host = mlab::ns::GetHostForToolAndMetroAndFamily(
        "npad", "ath", SOCKETFAMILY_IPV4);
    LOG(INFO, "ipv4 host: %s -> %s", host.original_hostname.c_str(),
        host.resolved_ips.begin()->c_str());
    EXPECT_EQ(0U, host.original_hostname.find(prefix));
    EXPECT_EQ(prefix.length() + 1, host.original_hostname.find("v4"));
    EXPECT_EQ(prefix.length() + 4, host.original_hostname.find("ath"));
    EXPECT_EQ(host.original_hostname.length() - suffix.length(),
              host.original_hostname.find(suffix));
  }

  // IPv6
  {
    mlab::Host host = mlab::ns::GetHostForToolAndMetroAndFamily(
        "npad", "ath", SOCKETFAMILY_IPV6);
    LOG(INFO, "ipv6 host: %s -> %s", host.original_hostname.c_str(),
        host.resolved_ips.begin()->c_str());
    EXPECT_EQ(0U, host.original_hostname.find(prefix));
    EXPECT_EQ(prefix.length() + 1, host.original_hostname.find("v6"));
    EXPECT_EQ(prefix.length() + 4, host.original_hostname.find("ath"));
    EXPECT_EQ(host.original_hostname.length() - suffix.length(),
              host.original_hostname.find(suffix));
  }
}

}  // namespace mlab
