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

#ifndef _MLAB_HTTP_H_
#define _MLAB_HTTP_H_

#include <stdint.h>
#include <string>

namespace mlab {
namespace http {

// HTTP GET for |href| on host |hostname|. Returns the first |response_length|
// bytes as a string.
std::string Get(const std::string& hostname,
                const std::string& href,
                size_t response_length);

// HTTP GET for |href| on host |hostname| at |port|. Returns the first
// |response_length| bytes as a string.
std::string Get(const std::string& hostname,
                uint16_t port,
                const std::string& href,
                size_t response_length);

}  // namespace http
}  // namespace mlab

#endif  // _MLAB_HTTP_H_

