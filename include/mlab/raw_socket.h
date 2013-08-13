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

#ifndef _MLAB_RAW_SOCKET_H_
#define _MLAB_RAW_SOCKET_H_

#include "mlab/socket.h"

namespace mlab {

class RawSocket : public Socket {
 public:
  static RawSocket* Create();  // default IPPROTO_IP, family IPv4
  static RawSocket* Create(SocketType type, SocketFamily family);

  // See |Create| for details. On failure, this version FATALs.
  static RawSocket* CreateOrDie();
  static RawSocket* CreateOrDie(SocketType type, SocketFamily family);

  virtual ~RawSocket();

  virtual int Connect(const Host& host);
  virtual int Bind(const Host& host);

  virtual bool Send(const Packet& bytes, ssize_t *num_bytes) const;
  virtual void SendToOrDie(const Host& host, const Packet& bytes,
                           ssize_t *num_bytes) const;
  virtual bool SendTo(const Host& host, const Packet& bytes,
                      ssize_t *num_bytes) const;

  virtual Packet Receive(size_t count, ssize_t *num_bytes);
  virtual Packet ReceiveFromOrDie(size_t count, Host* host);
  virtual Packet ReceiveFrom(size_t count, Host* host, ssize_t *num_bytes);

  bool SetIPHDRINCL();  // only effective for IPv4, however won't fail on v6.

 private:
  RawSocket(SocketType type, SocketFamily family);
};

}  // namespace mlab

#endif  // _MLAB_RAW_SOCKET_H_
