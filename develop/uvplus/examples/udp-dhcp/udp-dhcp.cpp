#include <iostream>
#include <memory>

#include <uvp.hpp>

uvp::uv::BufT makeDiscoverMsg() {
  uvp::uv::BufT buffer = uvp::allocBuf(256);

  // BOOTREQUEST
  buffer.base[0] = 0x1;
  // HTYPE ethernet
  buffer.base[1] = 0x1;
  // HLEN
  buffer.base[2] = 0x6;
  // HOPS
  buffer.base[3] = 0x0;
  // XID 4 bytes
  buffer.base[4] = (unsigned int)random();
  // SECS
  buffer.base[8] = 0x0;
  // FLAGS
  buffer.base[10] = 0x80;
  // CIADDR 12-15 is all zeros
  // YIADDR 16-19 is all zeros
  // SIADDR 20-23 is all zeros
  // GIADDR 24-27 is all zeros
  // CHADDR 28-43 is the MAC address, use your own
  buffer.base[28] = 0xe4;
  buffer.base[29] = 0xce;
  buffer.base[30] = 0x8f;
  buffer.base[31] = 0x13;
  buffer.base[32] = 0xf6;
  buffer.base[33] = 0xd4;
  // SNAME 64 bytes zero
  // FILE 128 bytes zero
  // OPTIONS
  // - magic cookie
  buffer.base[236] = 99;
  buffer.base[237] = (char)130;
  buffer.base[238] = 83;
  buffer.base[239] = 99;

  // DHCP Message type
  buffer.base[240] = 53;
  buffer.base[241] = 1;
  buffer.base[242] = 1; // DHCPDISCOVER

  // DHCP Parameter request list
  buffer.base[243] = 55;
  buffer.base[244] = 4;
  buffer.base[245] = 1;
  buffer.base[246] = 3;
  buffer.base[247] = 15;
  buffer.base[248] = 6;

  return buffer;
}

void onSend(uvp::Udp *socket, int status, uvp::uv::BufT bufs[], int nbufs) {
  if (status < 0) {
    std::cout << "send error: " << uvp::Error(status).strerror() << std::endl;
  }
  for (int i = 0; i < nbufs; ++i) {
    uvp::freeBuf(bufs[i]);
  }
  socket->close(nullptr);
}

void onRecv(uvp::Udp *socket, ssize_t nread, const uvp::uv::BufT *buf,
            const struct sockaddr *addr, unsigned flags) {
  if (nread < 0) {
    std::cout << "read error: " << uvp::Error(nread).strerror() << std::endl;
    socket->close(nullptr);
    return;
  }

  char sender[17] = {0};
  uvp::ip4Name((const sockaddr_in *)addr, sender, sizeof(sender) - 1);
  std::cout << "receive from: " << sender << std::endl;

  unsigned int *as_integer = (unsigned int *)buf->base;
  unsigned int ipbin = ntohl(as_integer[4]);
  unsigned char ip[4] = {0};
  int i;
  for (i = 0; i < 4; i++) {
    ip[i] = (ipbin >> i * 8) & 0xff;
  }
  std::cout << "Offered IP: " << (int)ip[3] << "." << (int)ip[2] << "."
            << (int)ip[1] << "." << (int)ip[0] << std::endl;

  socket->recvStop();
  socket->close(nullptr);
}

int main(int argc, char *argv[]) {
  uvp::initialize("./", "udp-dhcp", 1, 3);
  auto l = std::make_unique<uvp::LoopObject>();

  uvp::UdpObject sendSocket(l.get()), recvSocket(l.get());
  sockaddr_in recvAddr = {0};
  uvp::ip4Addr("0.0.0.0", 68, &recvAddr);
  recvSocket.bind((const sockaddr *)&recvAddr, UV_UDP_REUSEADDR);
  recvSocket.recvStart(nullptr, onRecv);

  sockaddr_in broadcastAddr = {0};
  uvp::ip4Addr("0.0.0.0", 0, &broadcastAddr);
  sendSocket.bind((const sockaddr *)&broadcastAddr, 0);
  sendSocket.setBroadcast(1);

  uvp::uv::BufT discoverMsg = makeDiscoverMsg();
  sockaddr_in sendAddr = {0};
  uvp::ip4Addr("255.255.255.255", 67, &sendAddr);
  sendSocket.send(&discoverMsg, 1, (const sockaddr *)&sendAddr, onSend);

  int r = l->run(UV_RUN_DEFAULT);
  r = l->close();

  LOG_INFO << "quit with return code: " << r;
  return r;
}
