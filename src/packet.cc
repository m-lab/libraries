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

#include "mlab/packet.h"

namespace mlab {

Packet::Packet(const std::vector<uint8_t>& data)
    : data_(data) {
}

Packet::Packet(const std::string& data)
    : data_(data.begin(), data.end()) {
  //  data_.push_back(0);
}

Packet::Packet(const char* buffer, size_t length)
    : data_(buffer, buffer + length) {
  //  data_.push_back(0);
}

}  // namespace mlab

