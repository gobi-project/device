#include <string.h>

#include <rest-engine.h>
#include <er-coap.h>
#include <er-coap-separate.h>
#include <er-coap-transactions.h>

void sendAnswer(void *data, void* resp);

static uint8_t big_msg[256];
static size_t big_msg_len = 0;

static coap_separate_t request_metadata[1];

void resource_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {
  if (*offset == 0) {
    if (coap_block1_handler(request, response, big_msg, &big_msg_len, 256)) {
      return;
    }
    printf("MSG: %s\n", big_msg);

    coap_separate_accept(request, request_metadata);
    erbium_status_code = NO_ERROR;
    request_metadata->block2_size = 32;

    coap_separate_resume(response, request_metadata, CREATED_2_01);
    coap_set_header_content_format(response, TEXT_PLAIN);
    coap_set_payload(response, big_msg, big_msg_len > 32 ? 32 : big_msg_len);
    if (big_msg_len > 32) *offset = 32;
  } else {
    if (*offset >= big_msg_len) {
      coap_set_status_code(response, BAD_OPTION_4_02);
      coap_set_payload(response, "BlockOutOfScope", 15);
      return;
    }

    memcpy(buffer, big_msg + *offset, 32);
    if (big_msg_len - *offset < preferred_size) {
      preferred_size = big_msg_len - *offset;
      *offset = -1;
    } else {
      *offset += preferred_size;
    }

    REST.set_header_content_type(response, TEXT_PLAIN);
    REST.set_response_payload(response, buffer, preferred_size);
  }
}

RESOURCE(resource, "rt=\"coap.test\";if=\"core.u\"", NULL, resource_handler, NULL, NULL);
