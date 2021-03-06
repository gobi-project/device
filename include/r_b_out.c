#include "flash.h"
#include "../../../contiki/tools/blaster/blaster.h"

#include <lib/sensors.h>
#include <gpio-util.h>

#include <rest-engine.h>
#include <string.h>

/* SENSOR ------------------------------------------------------------------ */

static int b_out_value(int type) {
  if (type == SENSORS_HW_INIT) return 0;
  return gpio_read(43);
}

static int b_out_configure(int type, int c) {
  switch (type) {
    case SENSORS_HW_INIT:
      gpio_set_pad_dir(43, PAD_DIR_OUTPUT);
      gpio_reg_set(GPIO_DATA_SEL1, 11);
      return 1;
    case SENSORS_ACTIVE:
      if (c) gpio_set(43);
      else gpio_reset(43);
      return 1;
    default:
      return 0;
  }
}

static int b_out_status(int type) {
  if (type == SENSORS_HW_INIT) return 0;
  return *GPIO_PAD_DIR1 & *GPIO_DATA_SEL1 & (1UL << 11);
}

SENSORS_SENSOR(b_out, "B_OUT", b_out_value, b_out_configure, b_out_status);

/* RESOURCE ---------------------------------------------------------------- */

void b_out_resource_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {
  int length = 0;

  if (REST.get_method_type(request) != METHOD_GET && *offset == 0) {
    const uint8_t *payload = 0;
    int len = 0;
    len = REST.get_request_payload(request, &payload);
    b_out.configure(SENSORS_ACTIVE, atoi(payload));
  }

  uint8_t source_string[LEN_SENML_BOUT]; // {"bn":"/swt","bu":"B","e":[{"v":"%d"}]}
  flash_getVar(source_string, RES_SENML_BOUT, LEN_SENML_BOUT);
  length = snprintf(buffer, REST_MAX_CHUNK_SIZE, source_string, b_out.value(SENSORS_ACTIVE));

  REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
  REST.set_response_payload(response, buffer, length);
}

RESOURCE(res_b_out, "rt=\"gobi.a.swt\";if=\"core.a\"", b_out_resource_handler, b_out_resource_handler, b_out_resource_handler, 0);
