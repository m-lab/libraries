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

#include "mlab/ns.h"

#include <sstream>
#include <string>

#include "json/reader.h"
#include "log.h"
#include "mlab/client_socket.h"
#include "mlab/http.h"

namespace mlab {
namespace ns {
namespace {

const char kHostname[] = "mlab-ns.appspot.com";
const char kStatusOK[] = "HTTP/1.1 200 OK";

// TODO(dominich): Right now mlab-ns returning !OK is a FATAL. Perhaps there's a
// better way to pass this info back to the caller.
std::string ParseResponse(const std::string& response) {
  std::istringstream stream(response);

  // Check the status.
  std::string status;
  std::getline(stream, status);

  status = status.substr(0, status.length() - 1);
  if (status != kStatusOK)
    LOG(FATAL, "Received status '%s' from mlab-ns.", status.c_str());

  // Extract HTTP headers
  std::map<std::string, std::string> headers;
  std::string header;
  std::getline(stream, header);
  while (header != "\r") {
    const size_t separator = header.find(": ");
    std::string key = header.substr(0, separator);
    const size_t value_start =  separator + 2;
    const size_t value_end = header.length() - 1;
    std::string value = header.substr(value_start, value_end - value_start);
    headers[key] = value;
    std::getline(stream, header);
  }

  // Ensure headers are as expected.
  if (headers["Content-Type"] != "application/json") {
    LOG(FATAL, "Expected Content-Type to be application/json but it is '%s'",
        headers["Content-Type"].c_str());
  }
  if (!headers["X-Google-AppEngine-AppId"].empty() &&
      headers["X-Google-AppEngine-AppId"] != "s~mlab-ns") {
    LOG(FATAL, "Unexpected AppEngine AppId: '%s'",
        headers["X-Google-AppEngine-AppId"].c_str());
  }

  size_t json_length;
  stream >> std::hex >> json_length;
  std::string json = std::string(std::istreambuf_iterator<char>(stream),
                                 std::istreambuf_iterator<char>());

  LOG(VERBOSE, "JSON: %s", json.c_str());

  Json::Value root;
  Json::Reader reader;

  if (!reader.parse(json, root))
    LOG(FATAL, "Failed to parse json: %s", json.c_str());

  // Take the FQDN and do the DNS lookup on the client side.
  return root.get("fqdn", "").asString();
}

Host GetHostForToolAndMetroAndFamilyWithPolicy(const std::string& tool,
                                               const std::string& metro,
                                               const std::string& policy,
                                               SocketFamily family) {
  std::string address_family;
  switch (family) {
    case SOCKETFAMILY_IPV4: address_family = "ipv4"; break;
    case SOCKETFAMILY_IPV6: address_family = "ipv6"; break;
    case SOCKETFAMILY_UNSPEC: break;
  }

  LOG(INFO,
      "Getting host for tool '%s', metro '%s', policy '%s', and family '%s'",
      tool.c_str(), metro.c_str(), policy.c_str(), address_family.c_str());
  const std::string href = tool + "?format=json" +
      (!metro.empty() ? "&metro=" + metro : "") +
      (!policy.empty() ? "&policy=" + policy : "") +
      (!address_family.empty() ? "&address_family=" + address_family : "");
  return Host(ParseResponse(http::Get(kHostname, href, 2048U)));
}

}  // namespace

Host GetHostForTool(const std::string& tool) {
  return GetHostForToolAndMetroAndFamilyWithPolicy(tool,
                                                   std::string(),
                                                   std::string(),
                                                   SOCKETFAMILY_UNSPEC);
}

Host GetRandomHostForTool(const std::string& tool) {
  return GetHostForToolAndMetroAndFamilyWithPolicy(tool,
                                                   std::string(),
                                                   "random",
                                                   SOCKETFAMILY_UNSPEC);
}

Host GetHostForToolAndFamily(const std::string& tool, SocketFamily family) {
  return GetHostForToolAndMetroAndFamilyWithPolicy(tool, std::string(),
                                                   std::string(), family);
}

Host GetRandomHostForToolAndFamily(const std::string& tool,
                                   SocketFamily family) {
  return GetHostForToolAndMetroAndFamilyWithPolicy(tool, std::string(),
                                                   "random", family);
}

Host GetHostForToolAndMetro(const std::string& tool, const std::string& metro) {
  return GetHostForToolAndMetroAndFamilyWithPolicy(tool, metro, std::string(),
                                                   SOCKETFAMILY_UNSPEC);
}

Host GetHostForToolAndMetroAndFamily(const std::string& tool,
                                     const std::string& metro,
                                     SocketFamily family) {
  return GetHostForToolAndMetroAndFamilyWithPolicy(tool, metro, std::string(),
                                                   family);
}

}  // namespace ns
}  // namespace mlab
