#include "dtls_content.h"

#include <string.h>
#include <arpa/inet.h>

typedef enum {
  con_length_0 = 0,
  con_length_8_bit = 1,
  con_length_16_bit = 2,
  con_length_24_bit = 3
} ContentLength;

typedef struct {
  ContentLength len:2;
  ContentType type:6;
  uint8_t payload[0];
} __attribute__ ((packed)) Content_t;

void *getContent(void *data, size_t len, ContentType type) {
    uint32_t pointer = 0;
    while (pointer < len) {
        Content_t *content = (Content_t *) (((uint8_t *) data) + pointer);
        if (content->type == type) {
            return content;
        }
        pointer += sizeof(Content_t) + content->len + getContentDataLen(content);
    }
    return 0;
}

ContentType getContentType(void *data) {
    return ((Content_t *) data)->type;
}

size_t getContentLen(void *data) {
    Content_t *content = (Content_t *) data;
    return sizeof(Content_t) + content->len + getContentDataLen(data);
}

size_t getContentDataLen(void *data) {
    uint32_t len;
    Content_t *content = (Content_t *) data;
    switch (content->len) {
        case con_length_8_bit:
            len = content->payload[0];
            break;
        case con_length_16_bit:
            len = content->payload[1];
            len += (content->payload[0] << 8);
            return len;
            break;
        case con_length_24_bit:
            len = content->payload[2];
            len += (content->payload[1] << 8);
            len += (content->payload[0] << 16);
            return len;
            break;
        default:
            len = 0;
    }
    return len;
}

void *getContentData(void *data) {
    if (((Content_t *) data)->len == con_length_0) {
        return 0;
    }
    return ((uint8_t *) data) + sizeof(Content_t) + ((Content_t *) data)->len;
}

size_t makeContent(void *dst, ContentType type, void *data, size_t len) {
    Content_t *content = (Content_t *) dst;
    content->type = type;
    if (len == 0) {
        content->len = con_length_0;
    } else {
        uint32_t rlen = htonl(len);
        uint8_t *l = (uint8_t *) &rlen;
        uint8_t i;
        for (i = 0; 1; i++) {
            if (l[i] != 0) {
                content->len = 4 - i;
                memcpy(content->payload, l + i, content->len);
                break;
            }
        }
    }
    memcpy(content->payload + content->len, data, len);
    return sizeof(Content_t) + content->len + len;
}
