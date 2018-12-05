#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include <ringbuffer.hpp>

/*
Packet := header + body
header := length + '|';
length := [0-9]+
body   := text

Example:

01A
12Hello World!
*/

char *parse_packet2(RingBuffer *buf) {
  if (buf->size() == 0) {
    return nullptr;
  }

  int head_len = 0;
  int body_len = 0;

  {
    char *body = buf->search('|');
    if (!body) {
      printf("[Partial Packet] header not ready, buffer %u\n", buf->size());
      return NULL;
    }
    head_len = buf->offset(body);
  }

  {
    char header[20];
    buf->peek(header, head_len);
    header[head_len] = '\0';
    body_len = atoi(header);
  }

  if (buf->size() < head_len + body_len) {
    printf("[Partial Packet] body not ready, buffer %u\n", buf->size());
    return NULL;
  }

  char *body = (char *)malloc(body_len + 1);
  if (body_len > 0) {
    buf->advance(head_len + 1);
    buf->read(body, body_len);
  }
  body[body_len] = '\0';

  return body;
}

#endif
