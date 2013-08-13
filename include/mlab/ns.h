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

#ifndef _MLAB_NS_H_
#define _MLAB_NS_H_

#include <string>

#include "mlab/host.h"
#include "mlab/socket_family.h"

namespace mlab {
namespace ns {

// Query m-lab-ns for the given |tool| and return a valid host close to client
// that can be used to open a connection.
Host GetHostForTool(const std::string& tool);

// Query m-lab-ns for the given |tool| and return a randomly selected valid host
// that can be used to open a connection.
Host GetRandomHostForTool(const std::string& tool);

// Query m-lab-ns for the given |tool| and address |family| and return a valid
// host close to client that can be used to open a connection.
Host GetHostForToolAndFamily(const std::string& tool, SocketFamily family);

// Query m-lab-ns for the given |tool| and address |family| and return a
// randomly selected valid host that can be used to open a connection.
Host GetRandomHostForToolAndFamily(const std::string& tool,
                                   SocketFamily family);

// Query m-lab-ns for the given |tool| and return a valid host close to the
// given |metro| area that can be used to open a connection.
Host GetHostForToolAndMetro(const std::string& tool, const std::string& metro);

// Query m-lab-ns for the given |tool| and address |family| and return a valid
// host close to the given |metro| area that can be used to open a connection.
Host GetHostForToolAndMetroAndFamily(const std::string& tool,
                                     const std::string& metro,
                                     SocketFamily family);
}
}

#endif  // _MLAB_NS_H_
