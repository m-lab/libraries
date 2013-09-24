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

#ifndef _MLAB_ACCEPTED_SOCKET_H_
#define _MLAB_ACCEPTED_SOCKET_H_

#if defined(OS_LINUX) || defined(OS_MACOSX) || defined(OS_ANDROID) || defined(OS_FREEBSD)
#include <arpa/inet.h>
#elif defined(OS_WINDOWS)
#include <WinSock2.h>
#include <WS2tcpip.h>
#else
#error Undefined platform
#endif

#include "mlab/host.h"
#include "socket.h"

namespace mlab {

// A socket that listens for connections.
class AcceptedSocket : public Socket {
 public:
  virtual ~AcceptedSocket();

  // Send |bytes| to a connected client. Sets |num_bytes| to the number of bytes
  // actually sent.
  virtual bool Send(const Packet& bytes, ssize_t *num_bytes) const;

  // Attempts to receive |count| bytes from connected clients. Returns the
  // received packet and sets |num_bytes| to the actual bytes received.
  virtual Packet Receive(size_t count, ssize_t *num_bytes);

 private:
  friend class ListenSocket;

  AcceptedSocket(int accepted_fd, SocketType type, SocketFamily family);

  // Valid for UDP sockets after first received packet.
  sockaddr_storage client_addr_;
  socklen_t client_addr_len_;
};

}  // namespace mlab

#endif  // _MLAB_ACCEPTED_SOCKET_H_
