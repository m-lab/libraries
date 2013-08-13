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

#if defined(OS_LINUX) || defined(OS_MACOSX)
#include <arpa/inet.h>
#include <netinet/ip_icmp.h>
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
#include "mlab/host.h"
#include "scoped_ptr.h"
#include "mlab/accepted_socket.h"
#include "mlab/client_socket.h"
#include "mlab/listen_socket.h"
#include "mlab/protocol_header.h"
#include "mlab/raw_socket.h"

namespace mlab {
namespace {

class RawSocketTest : public ::testing::Test {
  public:
    void SetGetBufsize(SocketType type, SocketFamily family) {
      RawSocket *sock = RawSocket::CreateOrDie(type, family);

      sock->SetSendBufferSize(900000);
      EXPECT_GE(sock->GetSendBufferSize(), 900000U);

      sock->SetRecvBufferSize(900000);
      EXPECT_GE(sock->GetRecvBufferSize(), 900000U);
    }

  protected:
    RawSocketTest() : payload("Hello, mlab!"),
                      header(20, 30, IPPROTO_ICMP, "127.0.0.1"),
                      local4lo("127.0.0.1"),
                      local6lo("::1"),
                      local4("127.0.0.1"),
                      local6("::1") {
      memset((char*)&header, 0, sizeof(IP4Header));
    }

    virtual ~RawSocketTest() { }

    virtual void SetUp() {
      char buffer[256] = {0};
      char* next = buffer;
      int len = 0;

      // ICMPv4 ECHO REQUEST/REPLY
      scoped_ptr<ICMP4Header>
          picmp4(new ICMP4Header(ICMP_ECHO, 0,
                                 0, htonl(0xabcd1234)));

      memcpy(buffer, (const char*)picmp4.get(), sizeof(ICMP4Header));
      next += sizeof(ICMP4Header);
      len += sizeof(ICMP4Header);
      memcpy(next, payload.buffer(), payload.length()-1);
      len += payload.length()-1;

          // ECHO REQUEST
      ICMP4Header *picmp = (struct ICMP4Header *)buffer;
      picmp->icmp_checksum = InternetCheckSum(buffer, len);
      icmp4packet = new Packet(buffer, len);  // one byte larger
          // ECHO REPLY
      picmp->icmp_checksum = 0;
      picmp->icmp_type = ICMP_ECHOREPLY;
      picmp->icmp_checksum = InternetCheckSum(buffer, len);
      icmp4packet_er = new Packet(buffer, len);

      // udp4packet
      scoped_ptr<UDPHeader>
          pudp(new UDPHeader(20136, 20135,
                             (payload.length()-1 + sizeof(UDPHeader)), 0));
      memset(buffer, 0, sizeof(buffer));
      memcpy(buffer, (const char*)pudp.get(), sizeof(UDPHeader));
      next = buffer + sizeof(UDPHeader);
      memcpy(next, payload.buffer(), payload.length()-1);
      len = payload.length()-1 + sizeof(UDPHeader);
      udppacket = new Packet(buffer, len);  // packet is one byte larger

      //  icmp 6
      scoped_ptr<ICMP6Header> picmp6(new ICMP6Header(128, 0,
                                                     0, htonl(0xcdef5678)));

      memcpy(buffer, (const char*)picmp6.get(), sizeof(ICMP6Header));
      next = buffer + sizeof(ICMP6Header);
      memcpy(next, payload.buffer(), payload.length()-1);
      len = sizeof(ICMP6Header);
      len += payload.length() - 1;
      icmp6packet = new Packet(buffer, len);

      ICMP6Header *picmp6_temp = (struct ICMP6Header *)buffer;
      picmp6_temp->icmp6_type = 129;  // echo reply
      icmp6packet_er = new Packet(buffer, len);  // still larger than 1
    }

  protected:
    Packet payload;
    IP4Header header;
    Packet* udppacket;
    Packet* icmp4packet;
    Packet* icmp4packet_er;
    Packet* icmp6packet;
    Packet* icmp6packet_er;
    Host local4lo;
    Host local6lo;
    Host local4;
    Host local6;
};

}  // namespace

TEST_F(RawSocketTest, IPv4Raw) {
  scoped_ptr<RawSocket> raw_socket_ptr(
      RawSocket::CreateOrDie(SOCKETTYPE_RAW, SOCKETFAMILY_IPV4));

  scoped_ptr<RawSocket> icmp_socket_ptr(
      RawSocket::CreateOrDie(SOCKETTYPE_ICMP, SOCKETFAMILY_IPV4));
  // test set IP_HDRINCL
  ASSERT_TRUE(raw_socket_ptr->SetIPHDRINCL());

  char buffer[256] = {0};
  int len = 0;
  Host recvaddr("192.168.0.1");

  // set up IPv4 header
  IP4Header header((sizeof(IP4Header) + icmp4packet->length()-1),
                    mlab_default_ttl, IPPROTO_ICMP, "127.0.0.1");
  header.id = htons(0x2013);

  // build whole packet
  memcpy(buffer, (char*)&header, sizeof(IP4Header));
  memcpy(buffer+sizeof(IP4Header), icmp4packet->buffer(),
         icmp4packet->length()-1);
  len = sizeof(IP4Header) + icmp4packet->length()-1;

  // send and test
  ssize_t num_bytes;
  raw_socket_ptr->SendToOrDie(local4lo, Packet(buffer, len), &num_bytes);
  EXPECT_EQ(len, num_bytes);

  // Recv, we won't recv the ECHO REQ packet we just sent, since
  // kernel won't pass ECHO REQ packet to raw socket
    // if we recv 5 icmp echo reply, yet none of ours, time to die...
  int magic_ttl = 5;
  Packet recvbuf("");
  const char *pos;

  while (magic_ttl) {
    ICMP4Header *hd;
    recvbuf = icmp_socket_ptr->ReceiveFromOrDie(len, &recvaddr);

    pos = recvbuf.buffer();
    pos += sizeof(IP4Header);  // in IPv4, we get whole IP packet
    hd = (ICMP4Header *)pos;
    if (hd->icmp_type == ICMP_ECHOREPLY) {
      if (ntohl(hd->icmp_rest) == 0xabcd1234)  // Fedex~
        break;

      magic_ttl--;
    }
  }

  EXPECT_GT(magic_ttl, 0);

  pos = recvbuf.buffer() + sizeof(IP4Header);
  pos += sizeof(ICMP4Header);  // come to payload
  recvbuf = Packet(pos, (len - sizeof(ICMP4Header) - sizeof(IP4Header)));

  // test sent packet integrity
  EXPECT_STREQ(payload.str().c_str(), recvbuf.str().c_str());
  LOG(INFO, "test rawsocket receivefrom set addr");
  EXPECT_STREQ(recvaddr.original_hostname.c_str(), "127.0.0.1");

  // test Raw socket sending UDP packets
  header = IP4Header((sizeof(IP4Header) + udppacket->length() - 1),
                     0, IPPROTO_UDP, "127.0.0.1");
  header.ttl = 0;
  header.id = htons(0x2013);
  memset(buffer, 0, sizeof(buffer));
  memcpy(buffer, (char*)&header, sizeof(IP4Header));
  memcpy(buffer+sizeof(IP4Header), udppacket->buffer(),
                       udppacket->length()-1);
  len = sizeof(IP4Header) + udppacket->length()-1;
  raw_socket_ptr->SendToOrDie(local4lo, Packet(buffer, len), &num_bytes);
  EXPECT_EQ(len, num_bytes);

  // recv ICMP dest unreach
  magic_ttl = 5;

  while (magic_ttl) {
    recvbuf = icmp_socket_ptr->ReceiveFromOrDie((len+
                                                 sizeof(ICMP4Header)+
                                                 sizeof(IP4Header)),
                                                &recvaddr);

    pos = recvbuf.buffer() + sizeof(IP4Header);
    ICMP4Header *hd = (ICMP4Header*)pos;
    if (hd->icmp_type == ICMP_DEST_UNREACH) {
      if (recvbuf.length() == (len + 1 +
                               sizeof(ICMP4Header) +
                               sizeof(IP4Header)) )
        break;

      magic_ttl--;
    }
  }

  EXPECT_GT(magic_ttl, 0);

     // test send data integrity
  uint16_t tempport = ntohs(*(uint16_t *)(pos+
                                          sizeof(ICMP4Header)+
                                          sizeof(IP4Header)));
  LOG(INFO, "test receive icmp dst unreach for udp.");
  EXPECT_EQ(tempport, 20136);
}

TEST_F(RawSocketTest, RawICMP) {
  // v4
  scoped_ptr<RawSocket> icmp_socket_ptr(
      RawSocket::CreateOrDie(SOCKETTYPE_ICMP, SOCKETFAMILY_IPV4));

  Host recvaddr("192.168.0.1");
  //  test send icmp echo reply and recv

  ssize_t num_bytes;
  icmp_socket_ptr->SendToOrDie(local4, *icmp4packet_er, &num_bytes);

  Packet rp = icmp_socket_ptr->ReceiveFromOrDie((icmp4packet_er->length() +
                                                 sizeof(IP4Header)), &recvaddr);

  ICMP4Header *icmp_ptr = (ICMP4Header *)(rp.buffer() + sizeof(IP4Header));

  int temp = (int)ntohl(icmp_ptr->icmp_rest);
  EXPECT_EQ(temp, (int)0xabcd1234);

  // Bind+Receive
  scoped_ptr<RawSocket> icmp_socket_ptr2(
      RawSocket::CreateOrDie(SOCKETTYPE_ICMP, SOCKETFAMILY_IPV4));
  EXPECT_EQ(0, icmp_socket_ptr->Bind(local4lo));  // 127.0.0.1
  EXPECT_EQ(0, icmp_socket_ptr2->Bind(local4));

  icmp_socket_ptr->SendToOrDie(local4, *icmp4packet_er, &num_bytes);

  rp = icmp_socket_ptr2->ReceiveOrDie(icmp4packet_er->length() +
                                      sizeof(IP4Header));
  icmp_ptr = (ICMP4Header *)(rp.buffer() + sizeof(IP4Header));
  temp = 0;
  temp = (int)ntohl(icmp_ptr->icmp_rest);

  LOG(INFO, "test Bind+Receive.");
  EXPECT_EQ(temp, (int)0xabcd1234);

  // Connect+Send
  EXPECT_EQ(0, icmp_socket_ptr->Connect(local4));
  icmp_socket_ptr->SendOrDie(*icmp4packet_er);

  rp = icmp_socket_ptr2->ReceiveOrDie(icmp4packet_er->length() +
                                      sizeof(IP4Header));
  icmp_ptr = (ICMP4Header *)(rp.buffer() + sizeof(IP4Header));
  temp = 0;
  temp = (int)ntohl(icmp_ptr->icmp_rest);

  LOG(INFO, "test Connect+Send.");
  EXPECT_EQ(temp, (int)0xabcd1234);

  // v6
  scoped_ptr<RawSocket> icmp6_socket_ptr(
      RawSocket::CreateOrDie(SOCKETTYPE_ICMP, SOCKETFAMILY_IPV6));

  icmp6_socket_ptr->SendToOrDie(local6lo, *icmp6packet, &num_bytes);
  rp = icmp6_socket_ptr->ReceiveFromOrDie(icmp6packet->length(), &recvaddr);

  ICMP6Header *temp6_ptr = (ICMP6Header *)(rp.buffer());

  EXPECT_EQ(temp6_ptr->icmp6_type, 129);
  EXPECT_EQ(temp6_ptr->icmp6_data32, ntohl(0xcdef5678));

  // v6 Bind+Receive
  scoped_ptr<RawSocket> icmp6_socket_ptr2(
      RawSocket::CreateOrDie(SOCKETTYPE_ICMP, SOCKETFAMILY_IPV6));

  EXPECT_EQ(0, icmp6_socket_ptr->Bind(local6lo));
  EXPECT_EQ(0, icmp6_socket_ptr2->Bind(local6));

  icmp6_socket_ptr->SendToOrDie(local6, *icmp6packet_er, &num_bytes);
  rp = icmp6_socket_ptr2->ReceiveOrDie(icmp6packet_er->length());

  temp6_ptr = (ICMP6Header *)(rp.buffer());

  LOG(INFO, "test v6 Bind+Receive.");
  EXPECT_EQ(temp6_ptr->icmp6_data32, ntohl(0xcdef5678));

  // v6 Connect+Send
  EXPECT_EQ(0, icmp6_socket_ptr->Connect(local6));
  icmp6_socket_ptr->SendOrDie(*icmp6packet_er);

  rp = icmp6_socket_ptr2->ReceiveOrDie(icmp6packet_er->length());
  temp6_ptr = (ICMP6Header *)(rp.buffer());
  LOG(INFO, "test v6 Connect+Send");
  EXPECT_EQ(temp6_ptr->icmp6_data32, ntohl(0xcdef5678));
}

TEST_F(RawSocketTest, TestSetBufsize) {
  LOG(INFO, "test set/get buffer size.");
  SetGetBufsize(SOCKETTYPE_RAW, SOCKETFAMILY_IPV4);
  SetGetBufsize(SOCKETTYPE_ICMP, SOCKETFAMILY_IPV4);
  SetGetBufsize(SOCKETTYPE_RAW, SOCKETFAMILY_IPV6);
  SetGetBufsize(SOCKETTYPE_ICMP, SOCKETFAMILY_IPV6);
}

TEST_F(RawSocketTest, TestGetAddrFamily) {
  std::string v4addr("8.8.8.8");
  std::string v6addr("::1");
  std::string notaddr("hello");

  EXPECT_EQ(SOCKETFAMILY_IPV4, GetSocketFamilyForAddress(v4addr));
  EXPECT_EQ(SOCKETFAMILY_IPV6, GetSocketFamilyForAddress(v6addr));
  EXPECT_EQ(SOCKETFAMILY_UNSPEC, GetSocketFamilyForAddress(notaddr));
}

TEST_F(RawSocketTest, TestProtocolHeader) {
  IP4Header hdr(40, 64, IPPROTO_ICMP, "127.0.0.1");
  IP6Header hdr6(80, 72, IPPROTO_UDP, "::1");

  EXPECT_EQ(hdr.SetSourceAddress("8.8.8.8"), 1);
  EXPECT_STREQ(hdr.GetSourceAddress().c_str(), "8.8.8.8");
  EXPECT_EQ(hdr.SetDestinationAddress("192.168.0.1"), 1);
  EXPECT_STREQ(hdr.GetDestinationAddress().c_str(), "192.168.0.1");

  EXPECT_EQ(hdr6.SetSourceAddress("::2"), 1);
  EXPECT_STREQ(hdr6.GetSourceAddress().c_str(), "::2");
  EXPECT_EQ(hdr6.SetDestinationAddress("::3"), 1);
  EXPECT_STREQ(hdr6.GetDestinationAddress().c_str(), "::3");
}

}  // namespace mlab
