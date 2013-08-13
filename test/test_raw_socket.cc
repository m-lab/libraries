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

#if defined(OS_LINUX) || defined(OS_MACOSX)
#include <arpa/inet.h>
#elif defined(OS_WINDOWS)
#else
#error Undefined platform.
#endif
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#if defined(OS_WINDOWS)
#include <winsock2.h>
#endif

#include "gtest/gtest.h"
#include "log.h"
#include "mlab/host.h"
#include "mlab/raw_socket.h"
#include "scoped_ptr.h"

namespace mlab {
namespace {

template<SocketType T, SocketFamily F>
struct TypeFamilyPair {
  static const SocketType type= T;
  static const SocketFamily family = F;
};

typedef TypeFamilyPair<SOCKETTYPE_RAW, SOCKETFAMILY_IPV4> IPIPv4;
typedef TypeFamilyPair<SOCKETTYPE_ICMP, SOCKETFAMILY_IPV4> ICMPIPv4;
typedef TypeFamilyPair<SOCKETTYPE_RAW, SOCKETFAMILY_IPV6> IPIPv6;
typedef TypeFamilyPair<SOCKETTYPE_ICMP, SOCKETFAMILY_IPV6> ICMPIPv6;

template<typename T>
class RawSocketTest : public ::testing::Test {
 protected:
  RawSocketTest() {
    LOG(INFO,
        "Create raw socket. Need root privilege. Protocol %s, family %s",
                T::type == SOCKETTYPE_RAW ? "RAW" : "ICMP",
                T::family == SOCKETFAMILY_IPV4 ? "IPv4" : "IPv6");
    mlab::scoped_ptr<mlab::RawSocket> raw_socket_ptr(
        mlab::RawSocket::Create(T::type, T::family));

    EXPECT_GT(raw_socket_ptr->raw(), -1);
  }

  virtual ~RawSocketTest() { }
};

}  // namespace

typedef ::testing::Types<IPIPv4, ICMPIPv4, IPIPv6, ICMPIPv6> TestParameters;
TYPED_TEST_CASE(RawSocketTest, TestParameters);

TYPED_TEST(RawSocketTest, SocketCreateDelete) {
}

}  // namespace mlab
