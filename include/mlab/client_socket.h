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

#ifndef _MLAB_CLIENT_SOCKET_H_
#define _MLAB_CLIENT_SOCKET_H_

#include "mlab/socket.h"

namespace mlab {

// A client-side socket that attempts to connect to a running server.
class ClientSocket : public Socket {
 public:
  // Create a socket to connect to |hostname| on |port|. Optionally supply the
  // |type| to be TCP or UDP (defaults to TCP) and the |family| to be IPv4 or
  // IPv6 (defaults to IPv4). On failure, this will return a NULL pointer. On
  // success, the caller is responsible for eventually deleting the socket.
  static ClientSocket* Create(const Host& hostname, uint16_t port);
  static ClientSocket* Create(const Host& hostname, uint16_t port,
                              SocketFamily family);
  static ClientSocket* Create(const Host& hostname, uint16_t port,
                              SocketType type);
  static ClientSocket* Create(const Host& hostname, uint16_t port,
                              SocketType type, SocketFamily family);
  static ClientSocket* Create(const Host& bindhost, uint16_t bindport,
                              const Host& connecthost, uint16_t connectport,
                              SocketType type, SocketFamily family);

  // See |Create| for details. On failure, this version FATALs.
  static ClientSocket* CreateOrDie(const Host& hostname, uint16_t port);
  static ClientSocket* CreateOrDie(const Host& hostname, uint16_t port,
                                   SocketFamily family);
  static ClientSocket* CreateOrDie(const Host& hostname, uint16_t port,
                                   SocketType type);
  static ClientSocket* CreateOrDie(const Host& hostname, uint16_t port,
                                   SocketType type, SocketFamily family);
  static ClientSocket* CreateOrDie(const Host& bindhost, uint16_t bindport,
                                   const Host& connecthost,
                                   uint16_t connectport,
                                   SocketType type, SocketFamily family);

  virtual ~ClientSocket();

  // Send |bytes| to the connected server. |num_bytes| is the number of bytes
  // actually sent.
  virtual bool Send(const Packet& bytes, ssize_t *num_bytes) const;

  // Attempts to receive |count| bytes from connected clients. Returns the
  // received packet. |num_bytes| is the number of bytes actually received.
  virtual Packet Receive(size_t count, ssize_t *num_bytes) const;

  // Attempts to accumulate |count| bytes from connected clients. Returns the
  // received packet. |num_bytes| is the number of bytes actually received.
  //TODO(dominic): Move this to socket (or at least accepted socket) and rename.
  virtual Packet ReceiveX(size_t count, ssize_t *num_bytes) const;

 private:
  ClientSocket(SocketType type, SocketFamily family);

  bool Bind(const Host& host, uint16_t port);
  bool Connect(const Host& host, uint16_t port);
};

}  // namespace mlab

#endif  // _MLAB_CLIENT_SOCKET_H_
