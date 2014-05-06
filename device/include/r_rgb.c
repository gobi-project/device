#include "storage.h"
#include "flash-store.h"

#include "contiki.h"
#include "lib/sensors.h"
#include "i2c.h"

#define RGB_ADDR  0x4

/* SENSOR ------------------------------------------------------------------ */

static int rgb_value(int type) {
  uint8_t rgb[] = { 255, 0 ,0 };

  /* transmit the register to start reading from */
  i2c_transmitinit(RGB_ADDR, 3, rgb);
  while (!i2c_transferred()); // wait for data to arrive

  return 0;
}

static int rgb_status(int type) {
  switch (type) {
    case SENSORS_ACTIVE:
    case SENSORS_READY:
      return 1; // fix?
      break;
  }

  return 0;
}

static int rgb_configure(int type, int c) {
  switch (type) {
    case SENSORS_HW_INIT:
      if (c) {
        i2c_disable;
      } else {
        i2c_enable();
      }
      return 1;
    default:
      return 0;
  }
}

SENSORS_SENSOR(rgb, "RGB", rgb_value, rgb_configure, rgb_status); // register the functions

/* RESOURCE ---------------------------------------------------------------- */

void rgb_resource_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {
  int length = 0;

  if (REST.get_method_type(request) != METHOD_GET) {
    const uint8_t *payload = 0;
    int len = 0;
    len = REST.get_request_payload(request, &payload);
    rgb.configure(SENSORS_ACTIVE, atoi(payload));
  }

  uint8_t source_string[LEN_SENML_LEDB]; // {"bn":"/rgb","bu":"B","e":[{"v":"%d"}]}
  nvm_getVar(source_string, RES_SENML_LEDB, LEN_SENML_LEDB);
  length = snprintf(buffer, REST_MAX_CHUNK_SIZE, source_string, rgb.value(SENSORS_ACTIVE));

  REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
  REST.set_response_payload(response, buffer, length);
}

RESOURCE(res_rgb, "rt=\"gobi.a.light.rgb\";if=\"core.a\"", rgb_resource_handler, rgb_resource_handler, rgb_resource_handler, 0);