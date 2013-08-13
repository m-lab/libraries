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

#ifndef _MLAB_MLAB_H_
#define _MLAB_MLAB_H_

#ifdef __cplusplus

#include <string>

#if defined(OS_WINDOWS)
#undef ERROR
#endif

namespace mlab {

enum LogSeverity {
  VERBOSE,
  INFO,
  WARNING,
  ERROR,
  FATAL
};

void Initialize(const std::string& client_id,
                const std::string& client_version);

void SetLogSeverity(LogSeverity s);

}  // namespace mlab

extern "C" {
#endif

#include "mlab/socket_family.h"
#include "mlab/socket_type.h"

// A zero-terminated linked list of resolved IP addresses that are returned by
// NS lookups.
struct Hostname {
#ifdef SWIG
  %immutable;
#endif  // SWIG
  const char* hostname;
  const char* ip_address;
  struct Hostname* next;
#ifdef SWIG
  %mutable;
#endif  // SWIG
};

void mlab_initialize(const char* client_id, const char* client_version);

// Query NS for an appropriate hostname for a given |tool| and optional |metro|
// and address |family|.
extern struct Hostname* mlab_ns_lookup_hostname_for_tool(const char* tool);
extern struct Hostname* mlab_ns_lookup_random_hostname_for_tool(
    const char* tool);
extern struct Hostname* mlab_ns_lookup_hostname_for_tool_and_family(
    const char* tool, enum SocketFamily family);
extern struct Hostname* mlab_ns_lookup_random_hostname_for_tool_and_family(
    const char* tool, enum SocketFamily family);
extern struct Hostname* mlab_ns_lookup_hostname_for_tool_and_metro(
    const char* tool, const char* metro);
extern struct Hostname* mlab_ns_lookup_hostname_for_tool_and_metro_and_family(
    const char* tool, const char* metro, enum SocketFamily family);

// Open a socket to listen on a given port using optional socket |type| and
// address |family|. Returns the socket fd on success and -1 on failure.
extern int mlab_listen_on(unsigned short port);
extern int mlab_listen_on_with_type(unsigned short port, enum SocketType type);
extern int mlab_listen_on_with_family(unsigned short port,
                                      enum SocketFamily family);
extern int mlab_listen_on_with_type_and_family(unsigned short port,
                                               enum SocketType type,
                                               enum SocketFamily family);

// Open a socket connected to the given hostname and port using optional socket
// |type| and address |family|. Returns the socket fd on success and -1 on
// failure.
extern int mlab_connect_to(struct Hostname* hostname, unsigned short port);
extern int mlab_connect_to_with_type(struct Hostname* hostname,
                                     unsigned short port,
                                     enum SocketType type);
extern int mlab_connect_to_with_family(struct Hostname* hostname,
                                       unsigned short port,
                                       enum SocketFamily family);
extern int mlab_connect_to_with_type_and_family(struct Hostname* hostname,
                                                unsigned short port,
                                                enum SocketType type,
                                                enum SocketFamily family);

// Send |count| bytes from |bytes| over |socket|. Returns number of bytes sent
// on success and -1 on failure.
extern int mlab_sendordie(int socket, const char* bytes, unsigned count);
extern int mlab_send(int socket, const char* bytes, unsigned count);

// Attempt to receive |count| bytes from |socket| and copies them into |bytes|.
// Returns number of bytes received on success and -1 on failure.
extern int mlab_recvordie(int socket, char* bytes, unsigned count);
extern int mlab_recv(int socket, char* bytes, unsigned count);

// Frees any memory allocated by calls to the above methods.
extern void mlab_shutdown();

#ifdef __cplusplus
}
#endif

#endif  // _MLAB_MLAB_H_
