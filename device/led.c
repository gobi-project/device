#include <lib/sensors.h>
#include <gpio-util.h>

#include <rest-engine.h>
#include <string.h>

/* SENSOR ------------------------------------------------------------------ */

static int value(int type) {
  if (type == SENSORS_HW_INIT) return 0;
  return gpio_read(42);
}

static int configure(int type, int c) {
  switch (type) {
    case SENSORS_HW_INIT:
      gpio_set_pad_dir(42, PAD_DIR_OUTPUT);
      gpio_reg_set(GPIO_DATA_SEL1, 10);
      return 1;
    case SENSORS_ACTIVE:
      if (c) gpio_set(42);
      else gpio_reset(42);
      return 1;
    default:
      return 0;
  }
}

static int status(int type) {
  if (type == SENSORS_HW_INIT) return 0;
  return *GPIO_PAD_DIR1 & *GPIO_DATA_SEL1 & (1UL << 10);
}

SENSORS_SENSOR(led, "LED", value, configure, status); // register the functions

/* RESOURCE ---------------------------------------------------------------- */

void led_resource_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {
  int length = 0;
  buffer[REST_MAX_CHUNK_SIZE-1] = 0;

  if (REST.get_method_type(request) != METHOD_GET) {
    const uint8_t *payload = 0;
    int len = 0;
    len = REST.get_request_payload(request, &payload);
    led.configure(SENSORS_ACTIVE, atoi(payload));
  }

  uint8_t source_string[LEN_SENML_BIN]; // {"bn":"%s","bu":"B","e":[{"v":"%d"}]}
  nvm_getVar(source_string, RES_SENML_BIN, LEN_SENML_BIN);
  length = snprintf(buffer, REST_MAX_CHUNK_SIZE, source_string, "/led", led.value(SENSORS_ACTIVE));

  REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
  REST.set_header_etag(response, (uint8_t *) &length, 1);
  REST.set_response_payload(response, buffer, length);
}

RESOURCE(res_led, "rt=\"led\";if=\"core.s\"", led_resource_handler, led_resource_handler, led_resource_handler, 0 );
