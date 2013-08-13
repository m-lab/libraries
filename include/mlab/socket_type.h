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

#ifndef _MLAB_SOCKET_TYPE_H_
#define _MLAB_SOCKET_TYPE_H_
#define _MLAB_SOCKET_TYPE_H_

#include <sys/socket.h>

enum SocketType {
  SOCKETTYPE_TCP = SOCK_STREAM,
  SOCKETTYPE_UDP = SOCK_DGRAM,
  SOCKETTYPE_RAW = SOCK_RAW,
  SOCKETTYPE_ICMP = -2  // special case for SOCK_RAW + IPPROTO_ICMP
};

#endif  //  _MLAB_SOCKET_TYPE_H_
