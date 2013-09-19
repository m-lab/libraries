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

#if defined(OS_LINUX) || defined(OS_MACOSX) || defined(OS_FREEBSD)
#include <arpa/inet.h>
#elif defined(OS_WINDOWS)
#else
#error Undefined platform.
#endif
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#if defined(OS_WINDOWS)
#include <winsock2.h>
#endif

#include "gtest/gtest.h"
#include "log.h"
#include "mlab/accepted_socket.h"
#include "mlab/client_socket.h"
#include "mlab/host.h"
#include "mlab/listen_socket.h"
#include "scoped_ptr.h"

namespace mlab {
namespace {

const char message_str[] = "hello";

// TODO: Test multiple connections to one server socket.

class SocketTestBase : public ::testing::Test {
 protected:
  static const Packet message;
  static const int port;

  SocketTestBase() {
    // Start up server thread.
    pthread_t server_thread_id;
    sem_init(&server_ready_, 0, 0);
    pthread_create(&server_thread_id, 0, &ServerThread, this);
    pthread_detach(server_thread_id);
  }

  virtual ~SocketTestBase() { }

  virtual void SetUp() {
    // Wait for server to be listening.
    sem_wait(&server_ready_);
  }

  virtual void Server() = 0;

  sem_t server_ready_;

 private:
  static void* ServerThread(void* that) {
    SocketTestBase* self = static_cast<SocketTestBase*>(that);
    self->Server();
    return NULL;
  }
};

// static
const Packet SocketTestBase::message((std::string(message_str)));

// static
const int SocketTestBase::port = 5000;

template<SocketType T, SocketFamily F>
struct TypeFamilyPair {
  static const SocketType type = T;
  static const SocketFamily family = F;

  static std::string ToString() {
    std::string str;
    switch (type) {
      case SOCKETTYPE_TCP: str += "TCP"; break;
      case SOCKETTYPE_UDP: str += "UDP"; break;
      default: ASSERT(false); break;
    };
    str += "|";
    switch (family) {
      case SOCKETFAMILY_IPV4: str += "IPv4"; break;
      case SOCKETFAMILY_IPV6: str += "IPv6"; break;
      default: ASSERT(false); break;
    };
    return str;
  }
};

template<typename T>
class SocketTest : public SocketTestBase {
 protected:
  static const mlab::Host localhost;

  SocketTest() : SocketTestBase() {
    ASSERT(T::family == SOCKETFAMILY_IPV4 || T::family == SOCKETFAMILY_IPV6);
  }

  virtual ~SocketTest() { }

  virtual void Server() {
    LOG(INFO, ">> Starting server...");
    mlab::scoped_ptr<mlab::ListenSocket> listen_socket(
        mlab::ListenSocket::CreateOrDie(port, T::type, T::family));

    LOG(INFO, ">> Bound...");
    sem_post(&server_ready_);
    listen_socket->SelectWithTimeout(5);
    LOG(INFO, ">> Selected!");
    mlab::scoped_ptr<mlab::AcceptedSocket> accepted_socket(
        listen_socket->AcceptOrDie());
    LOG(INFO, ">> Accepted!");

    Packet recv_str = accepted_socket->ReceiveOrDie(message.length());
    ASSERT_GT(recv_str.length(), 0U);
    LOG(INFO, ">> Received %s", recv_str.str().c_str());
    LOG(INFO, ">> Sending %s", recv_str.str().c_str());
    ssize_t num_send_bytes = accepted_socket->SendOrDie(recv_str);
    ASSERT_GT(num_send_bytes, 0);
    EXPECT_EQ(recv_str.length(), static_cast<size_t>(num_send_bytes));
  }

  void SendAndReceive() {
    // Send a message.
    const mlab::Host& host = localhost;
    LOG(INFO, "<< Connecting %s %d", host.original_hostname.c_str(), port);
    mlab::scoped_ptr<mlab::ClientSocket> client_socket(
        mlab::ClientSocket::CreateOrDie(host, port, T::type, T::family));

    LOG(INFO, "<< Sending %s", message_str);
    EXPECT_EQ(message.length(),
              static_cast<size_t>(client_socket->SendOrDie(message)));

    // Recv it back.
    size_t bytecount = message.length();
    LOG(INFO, "<< Receiving %zu bytes.", bytecount);
    Packet buffer = client_socket->ReceiveOrDie(bytecount);
    LOG(INFO, "<< Received %s", buffer.buffer());
    EXPECT_STREQ(message_str, buffer.str().c_str());
  }
};

typedef TypeFamilyPair<SOCKETTYPE_TCP, SOCKETFAMILY_IPV4> TCPIPv4;
typedef TypeFamilyPair<SOCKETTYPE_UDP, SOCKETFAMILY_IPV4> UDPIPv4;
typedef TypeFamilyPair<SOCKETTYPE_TCP, SOCKETFAMILY_IPV6> TCPIPv6;
typedef TypeFamilyPair<SOCKETTYPE_UDP, SOCKETFAMILY_IPV6> UDPIPv6;

// static
template<>
const mlab::Host SocketTest<TCPIPv4>::localhost("127.0.0.1");

// static
template<>
const mlab::Host SocketTest<UDPIPv4>::localhost("127.0.0.1");

// static
template<>
const mlab::Host SocketTest<TCPIPv6>::localhost("::1");

// static
template<>
const mlab::Host SocketTest<UDPIPv6>::localhost("::1");

}  // namespace

typedef ::testing::Types<TCPIPv4, UDPIPv4, TCPIPv6, UDPIPv6> TestParameters;
TYPED_TEST_CASE(SocketTest, TestParameters);

TYPED_TEST(SocketTest, SendAndReceive) {
  this->SendAndReceive();
}

TEST(SocketTest, BufferSize) {
  mlab::scoped_ptr<mlab::ListenSocket> listen_socket(
      mlab::ListenSocket::CreateOrDie(1234, SOCKETTYPE_TCP, SOCKETFAMILY_IPV4));
  listen_socket->SetSendBufferSize(900000U);
  EXPECT_GE(listen_socket->GetSendBufferSize(), 900000U);

  listen_socket->SetRecvBufferSize(600000U);
  EXPECT_GE(listen_socket->GetRecvBufferSize(), 600000U);
}

}  // namespace mlab
