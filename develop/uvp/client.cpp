/*****************************************************
 * A TCP socket client that receive Partial Packet
 * and Merged Packets from TCP connection.
 * @author: ideawu
 * @link: http://www.ideawu.net/
 *****************************************************/
#include "inc.h"

int main(int argc, char **argv) {
  if (argc <= 2) {
    printf("Usage: %s ip port\n", argv[0]);
    exit(0);
  }

  const char *ip = argv[1];
  int port = atoi(argv[2]);

  int sock = sock_connect(ip, port);
  if (sock == -1) {
    printf("error: %s\n", strerror(errno));
    exit(0);
  }
  printf("connected to %s:%d\n", ip, port);

  RingBuffer buf(23);
  while (1) {
    // 可以优化成直接网络读到buf中，不需要tmp
    char tmp[23];
    int len = read(sock, tmp, sizeof(tmp));
    if (len <= 0) {
      printf("receive %d, exit.\n", len);
      exit(0);
    }
		if (len > buf.capacity()) {
      printf("ring buffer too small. need: %d, exit.\n", len);
      exit(0);
    }
    buf.write(tmp, len);
    int n = 0;
    while (1) {
      char *msg = parse_packet2(&buf);
      if (!msg) {
        break;
      }
      n++;

      printf("< %s\n", msg);
      if (n > buf.capacity()) {
        printf("    [Merged Packet]\n");
        exit(0);
      }
      free(msg);
    }
  }

  return 0;
}
