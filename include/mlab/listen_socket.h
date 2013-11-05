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

#ifndef _MLAB_LISTEN_SOCKET_H_
#define _MLAB_LISTEN_SOCKET_H_

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
class AcceptedSocket;

// A socket that listens for connections.
class ListenSocket : public Socket {
 public:
  // Create a socket to bind to and listen on |port|. Optionally supply the
  // |type| to be TCP or UDP (defaults to TCP) and the |family| to be IPv4 or
  // IPv6 (defaults to IPv4). On failure, this will return a NULL pointer. On
  // success, the caller is responsible for eventually deleting the socket.
  static ListenSocket* Create(uint16_t port);
  static ListenSocket* Create(uint16_t port, SocketFamily family);
  static ListenSocket* Create(uint16_t port, SocketType type);
  static ListenSocket* Create(uint16_t port, SocketType type,
                              SocketFamily family);

  // See |Create| for details. On failure, this version FATALs.
  static ListenSocket* CreateOrDie(uint16_t port);
  static ListenSocket* CreateOrDie(uint16_t port, SocketFamily family);
  static ListenSocket* CreateOrDie(uint16_t port, SocketType type);
  static ListenSocket* CreateOrDie(uint16_t port, SocketType type,
                                   SocketFamily family);

  virtual ~ListenSocket();

  // Start blocking until a client connects or |timeout| seconds have passed.
  void Select();
  void SelectWithTimeout(uint32_t timeout);

  // Accept the selected connection. Note, this may fail if the client
  // disconnects after Select succeeds. On success, a valid AcceptedSocket
  // pointer is returned. Caller is responsible for deleting this.
  AcceptedSocket* Accept() const;

  // See |Accept| for details. On failure, this version FATALs.
  AcceptedSocket* AcceptOrDie() const;

  virtual bool Send(const Packet&, ssize_t*) const;
  virtual Packet Receive(size_t, ssize_t*) const;

 private:
  ListenSocket(uint16_t port, SocketType type, SocketFamily family);

  void Start(uint16_t port);
};

}  // namespace mlab

#endif  // _MLAB_LISTEN_SOCKET_H_
