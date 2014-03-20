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
  if (coap_block1_handler(request, response, big_msg, &big_msg_len, 256)) {
      return;
  }

  printf("MSG: %s\n", big_msg);

  coap_separate_accept(request, request_metadata);
  sendAnswer(NULL, request);
}

void sendAnswer(void *data, void* resp) {
  request_metadata->block2_size = 32;

  uint8_t buffer[32];
  memcpy(buffer, big_msg + (request_metadata->block2_num * 32), 32);
  uint32_t read = 32;
  if ((request_metadata->block2_num + 1) * 32 >= big_msg_len) {
    read = big_msg_len % 32;
    if (read == 0) read = 32;
  } else {
    read = 0;
  }

  coap_transaction_t *transaction = NULL;
  if ( (transaction = coap_new_transaction(request_metadata->mid, &request_metadata->addr, request_metadata->port)) ) {
    coap_packet_t response[1];
    coap_separate_resume(response, request_metadata, CREATED_2_01);
    coap_set_header_content_format(response, TEXT_PLAIN);
    coap_set_payload(response, buffer, read == 0 ? request_metadata->block2_size : read);
    coap_set_header_block2(response, request_metadata->block2_num, read == 0 ? 1 : 0, request_metadata->block2_size);
    // TODO Warning: No check for serialization error.
    transaction->packet_len = coap_serialize_message(response, transaction->packet);
    transaction->callback = (read == 0 ? &sendAnswer : NULL);
    coap_send_transaction(transaction);
    request_metadata->block2_num++;
  }
}

RESOURCE(resource, "rt=\"coap.test\";if=\"core.u\"", NULL, resource_handler, NULL, NULL);
