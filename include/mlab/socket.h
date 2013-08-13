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

#ifndef _MLAB_SOCKET_H_
#define _MLAB_SOCKET_H_

#include <stdint.h>

#include "mlab/host.h"
#include "mlab/packet.h"
#include "mlab/socket_family.h"
#include "mlab/socket_type.h"

namespace mlab {

class Socket {
 public:
  virtual ~Socket();

  virtual bool Send(const Packet& bytes, ssize_t* num_bytes) const = 0;
  virtual Packet Receive(size_t count, ssize_t* num_bytes) = 0;

  ssize_t SendOrDie(const Packet& bytes) const;
  Packet ReceiveOrDie(size_t count);

  bool SetSendBufferSize(size_t size);
  bool SetRecvBufferSize(size_t size);
  size_t GetSendBufferSize() const;
  size_t GetRecvBufferSize() const;

  int raw() const { return fd_; }

 protected:
  Socket(SocketType type, SocketFamily family);

  void CreateSocket();
  void DestroySocket();

  SocketType type() const {
    return type_ == SOCKETTYPE_ICMP ? SOCKETTYPE_RAW : type_;
  }

  int fd_;
  SocketFamily family_;
  int protocol_;

 private:
  enum BufferType {
    BUFFERTYPE_SEND,
    BUFFERTYPE_RECV
  };

  bool SetBufferSize(const size_t& size, BufferType type);
  size_t GetBufferSize(BufferType type) const;

  SocketType type_;
};

}  // namespace mlab

#endif  // _MLAB_SOCKET_H_
