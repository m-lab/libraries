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

#include "mlab/http.h"

#include <sstream>

#include "log.h"
#include "mlab/client_socket.h"
#include "scoped_ptr.h"

namespace mlab {

extern const std::string& get_user_agent();

namespace http {
namespace {

const uint16_t default_port = 80;

}  // namespace

std::string Get(const std::string& hostname,
                const std::string& href,
                size_t response_length) {
  return Get(hostname, default_port, href, response_length);
}

std::string Get(const std::string& hostname,
                uint16_t port,
                const std::string& href,
                size_t response_length) {
  // Use a raw socket to send the HTTP request instead of using curl to reduce
  // dependencies and make cross-platform support simpler.
  Host host(hostname);

  scoped_ptr<ClientSocket> socket(ClientSocket::CreateOrDie(host,
                                                            default_port));

  std::stringstream request;
  request << "GET http://" << hostname << "/" << href << " HTTP/1.1\r\n";
  request << "Host: " << *host.resolved_ips.begin() << ":" << default_port <<
             "\r\n";
  request << "User-Agent: " << get_user_agent() << "\r\n";
  request << "\r\n";
  LOG(VERBOSE, "Sending request: %s", request.str().c_str());
  Packet p(request.str());
  socket->SendOrDie(p);

  // TODO(dominich): Better management of receive buffer than a fixed guess of
  // the length.
  return socket->ReceiveOrDie(response_length).str();;
}

}  // namespace http
}  // namespace mlab
