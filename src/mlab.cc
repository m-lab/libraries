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

#include "mlab/mlab.h"

#if defined(OS_LINUX) || defined(OS_MACOSX) || defined(OS_ANDROID)
#include <errno.h>
#include <sys/utsname.h>
#endif

#if defined(OS_LINUX)
#include <malloc.h>
#endif

#include <string.h>

#include <string>
#include <vector>

#include "log.h"
#include "mlab/accepted_socket.h"
#include "mlab/client_socket.h"
#include "mlab/listen_socket.h"
#include "mlab/ns.h"

using mlab::AcceptedSocket;
using mlab::ClientSocket;
using mlab::Host;
using mlab::IPAddresses;
using mlab::ListenSocket;
using mlab::Socket;

namespace {

std::vector<Socket*> sockets;
std::vector<Hostname*> hostnames;

// TODO: Change this so that Hostname (bad name) is a hostname and list of ips
// rather than a list of hostnames and ips.
Hostname* CreateHostnameListFromHost(const Host& host) {
  LOG(mlab::VERBOSE, "Creating list of %zu entries.", host.resolved_ips.size());
  Hostname* hostname_list = static_cast<Hostname*>(
      malloc(sizeof(Hostname) * host.resolved_ips.size()));

  IPAddresses::const_iterator iter = host.resolved_ips.begin();
  for (size_t i = 0; iter != host.resolved_ips.end(); ++iter, ++i) {
    hostname_list[i].hostname = host.original_hostname.c_str();
    hostname_list[i].ip_address = iter->c_str();
    LOG(mlab::VERBOSE, "  [%zu] %s", i, hostname_list[i].ip_address);
    if (i < host.resolved_ips.size() - 1)
      hostname_list[i].next = &(hostname_list[i+1]);
    else
      hostname_list[i].next = 0;
  }
  hostnames.push_back(hostname_list);
  return hostname_list;
}

int AcceptAndStore(ListenSocket* socket) {
  sockets.push_back(socket);
  AcceptedSocket* accepted = socket->Accept();
  if (accepted == NULL)
    return -1;
  sockets.push_back(accepted);
  return socket->raw();
}

std::string Platform() {
  std::string platform_str;
#if defined(OS_LINUX) || defined(OS_MACOSX) || defined(OS_ANDROID)
  utsname uts;
  if (uname(&uts) == 0) {
    LOG(mlab::VERBOSE, "Sysname:  %s", uts.sysname);
    LOG(mlab::VERBOSE, "Nodename: %s", uts.nodename);
    LOG(mlab::VERBOSE, "Release:  %s", uts.release);
    LOG(mlab::VERBOSE, "Version:  %s", uts.version);
    LOG(mlab::VERBOSE, "Machine:  %s", uts.machine);

    platform_str.assign(uts.sysname);
    platform_str += " (" + std::string(uts.release) + ")";
  } else {
    LOG(mlab::WARNING, "Failed to get uname: %s", strerror(errno));
    platform_str.assign("Linux");
  }
#else
#error Unknown platform.
#endif
  return platform_str;
}

}  // namespace

namespace mlab {
namespace { std::string user_agent; }

const std::string& get_user_agent() { return user_agent; }

void Initialize(const std::string& client_id,
                const std::string& client_version) {
  user_agent = "M-Lab " + client_id + " (" + client_version + ") " + Platform();
}
}  // namespace mlab

void mlab_initialize(const char* client_id, const char* client_version) {
  mlab::Initialize(client_id, client_version);
}

Hostname* mlab_ns_lookup_hostname_for_tool(const char* tool) {
  return CreateHostnameListFromHost(
      mlab::ns::GetHostForTool(std::string(tool)));
}

Hostname* mlab_ns_lookup_random_hostname_for_tool(const char* tool) {
  return CreateHostnameListFromHost(
      mlab::ns::GetRandomHostForTool(std::string(tool)));
}

Hostname* mlab_ns_lookup_hostname_for_tool_and_family(const char* tool,
                                                      SocketFamily family) {
  return CreateHostnameListFromHost(
      mlab::ns::GetHostForToolAndFamily(std::string(tool), family));
}

Hostname* mlab_ns_lookup_random_hostname_for_tool_and_family(
    const char* tool, SocketFamily family) {
  return CreateHostnameListFromHost(
      mlab::ns::GetRandomHostForToolAndFamily(std::string(tool), family));
}

Hostname* mlab_ns_lookup_hostname_for_tool_and_metro(const char* tool,
                                                     const char* metro) {
  return CreateHostnameListFromHost(
      mlab::ns::GetHostForToolAndMetro(std::string(tool), std::string(metro)));
}

Hostname* mlab_ns_lookup_hostname_for_tool_and_metro_and_family(
    const char* tool, const char* metro, SocketFamily family) {
  return CreateHostnameListFromHost(mlab::ns::GetHostForToolAndMetroAndFamily(
      std::string(tool), std::string(metro), family));
}

// The mlab_listen_on* functions could be consolidated, however the defaults are
// already set in C++ so call through to those directly.
int mlab_listen_on(unsigned short port) {
  return AcceptAndStore(ListenSocket::CreateOrDie(port));
}

int mlab_listen_on_with_type(unsigned short port, SocketType type) {
  return AcceptAndStore(ListenSocket::CreateOrDie(port, type));
}

int mlab_listen_on_with_family(unsigned short port, SocketFamily family) {
  return AcceptAndStore(ListenSocket::CreateOrDie(port, family));
}

int mlab_listen_on_with_type_and_family(unsigned short port, SocketType type,
                                        SocketFamily family) {
  return AcceptAndStore(ListenSocket::CreateOrDie(port, type, family));
}

// The mlab_connect_to* functions could be consolidated, however the defaults
// are already set in C++ so call through to those directly.
int mlab_connect_to(Hostname* hostname, unsigned short port) {
  // This causes a double lookup as we're recreating the Host.
  while (hostname != 0) {
    ClientSocket* socket = ClientSocket::CreateOrDie(Host(hostname->ip_address),
                                                     port);
    if (socket->raw() != -1) {
      sockets.push_back(socket);
      return socket->raw();
    }
    hostname = hostname->next;
  }
  return -1;
}

int mlab_connect_to_with_type(Hostname* hostname, unsigned short port,
                              SocketType type) {
  // This causes a double lookup as we're recreating the Host.
  while (hostname != 0) {
    ClientSocket* socket = ClientSocket::CreateOrDie(Host(hostname->ip_address),
                                                     port, type);
    if (socket->raw() != -1) {
      sockets.push_back(socket);
      return socket->raw();
    }
    hostname = hostname->next;
  }
  return -1;
}

int mlab_connect_to_with_family(Hostname* hostname, unsigned short port,
                                SocketFamily family) {
  // This causes a double lookup as we're recreating the Host.
  while (hostname != 0) {
    ClientSocket* socket = ClientSocket::CreateOrDie(Host(hostname->ip_address),
                                                     port, family);
    if (socket->raw() != -1) {
      sockets.push_back(socket);
      return socket->raw();
    }
    hostname = hostname->next;
  }
  return -1;
}

int mlab_connect_to_with_type_and_family(Hostname* hostname,
                                         unsigned short port,
                                         SocketType type,
                                         SocketFamily family) {
  // This causes a double lookup as we're recreating the Host.
  while (hostname != 0) {
    ClientSocket* socket = ClientSocket::CreateOrDie(Host(hostname->ip_address),
                                                     port, type, family);
    if (socket->raw() != -1) {
      sockets.push_back(socket);
      return socket->raw();
    }
    hostname = hostname->next;
  }
  return -1;
}

int mlab_send(int socket, const char* bytes, unsigned count) {
  for (std::vector<Socket*>::iterator it = sockets.begin(); it != sockets.end();
       ++it) {
    if ((*it)->raw() == socket) {
      mlab::Packet p(bytes, count);
      ssize_t num_bytes;
      (*it)->Send(p, &num_bytes);
      return num_bytes;
    }
  }
  return -1;
}

int mlab_sendordie(int socket, const char* bytes, unsigned count) {
  int rt = mlab_send(socket, bytes, count);
  if (rt < 0)
    LOG(mlab::FATAL, "send fails.");

  return rt;
}

int mlab_recv(int socket, char* bytes, unsigned count) {
  for (std::vector<Socket*>::iterator it = sockets.begin(); it != sockets.end();
       ++it) {
    if ((*it)->raw() == socket) {
      ssize_t num_bytes;
      mlab::Packet recv = (*it)->Receive(count, &num_bytes);
      if (num_bytes > 0)
        strncpy(bytes, recv.buffer(), recv.length());

      return num_bytes;
    }
  }
  return -1;
}

int mlab_recvordie(int socket, char* bytes, unsigned count) {
  int rt = mlab_recv(socket, bytes, count);
  if (rt < 0)
    LOG(mlab::FATAL, "recv fails.");

  return rt;
}

void mlab_shutdown() {
  for (std::vector<Hostname*>::iterator it = hostnames.begin();
       it != hostnames.end(); ++it) {
    free(*it);
  }
  hostnames.clear();
  for (std::vector<Socket*>::iterator it = sockets.begin(); it != sockets.end();
       ++it) {
    delete *it;
  }
  sockets.clear();
}
