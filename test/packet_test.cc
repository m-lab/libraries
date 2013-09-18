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

#include "gtest/gtest.h"
#include "mlab/packet.h"
#include "log.h"

namespace mlab {
namespace {
const char buffer[] = "hello world";

template<typename T, typename U>
void ExpectVectorEqual(const std::vector<T>& expected,
                       const std::vector<U>& actual) {
  EXPECT_EQ(expected.size(), actual.size());
  for (size_t i = 0; i < expected.size(); ++i)
    EXPECT_EQ(expected[i], actual[i]);
}
}  // namespace

TEST(PacketTest, FromCharArray) {
  Packet p(buffer, strlen(buffer));

  std::vector<uint8_t> expected_p(strlen(buffer));
  for (size_t i = 0; i < expected_p.size(); ++i)
    expected_p[i] = buffer[i];
  // Packet no longer contains trailing NUL
  // expected_p.push_back(0);

  ExpectVectorEqual(expected_p, p.data());
}

TEST(PacketTest, ToCharArray) {
  Packet p(buffer, strlen(buffer));
  const char* p_str = p.str().c_str();
  EXPECT_STREQ(buffer, p_str);
}

TEST(PacketTest, FromString) {
  Packet p(std::string(buffer, strlen(buffer)));

  std::vector<uint8_t> expected_p(strlen(buffer));
  for (size_t i = 0; i < expected_p.size(); ++i)
    expected_p[i] = buffer[i];
  // Packet no longer contains trailing NUL
  // expected_p.push_back(0);

  ExpectVectorEqual(expected_p, p.data());
}

TEST(PacketTest, ToString) {
  Packet p(std::string(buffer, strlen(buffer)));
  EXPECT_STREQ(std::string(buffer).c_str(), p.str().c_str());
}

TEST(PacketTest, FromStringOfNulls) {
  std::string message(128, '0');
  Packet p(message);
  EXPECT_GT(p.length(), 0U);
  EXPECT_EQ(message.size(), p.length());
}

}  // namespace mlab
