#include "flash-store.h"
#include "storage.h"

#include <lib/sensors.h>
#include <pwm.h>

#include <rest-engine.h>
#include <string.h>

/* SENSOR ------------------------------------------------------------------ */

static uint32_t stored_value = 0;

static int led_dim_value(int type) {
  return stored_value;
}

static int led_dim_configure(int type, int c) {
  switch (type) {
    case SENSORS_HW_INIT:
      pwm_init(TMR2, 300, 1);
      return 1;
    case SENSORS_ACTIVE:
      stored_value = c;
      c = c + 7;
      c = ((c*c*c*c)>>12)+1;
      if (c > 0 && c < 65536 ) {
        pwm_duty(TMR2, c);
        return 1;
      }
      return 1;
    default:
      return 0;
  }
}

static int led_dim_status(int type) {
  if (type == SENSORS_HW_INIT) return 0;
  return TMR0->ENBL & (1 << TMR_NUM(TMR2));
}

SENSORS_SENSOR(led_dim, "LED_D", led_dim_value, led_dim_configure, led_dim_status);

/* RESOURCE ---------------------------------------------------------------- */

void led_dim_resource_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {
  int length = 0;

  if (REST.get_method_type(request) != METHOD_GET) {
    const uint8_t *payload = 0;
    int len = 0;
    len = REST.get_request_payload(request, &payload);
    led_dim.configure(SENSORS_ACTIVE, atoi(payload));
  }

  uint8_t source_string[LEN_SENML]; // {"bn":"%s","bu":"%s","e":[{"v":"%d"}]}
  nvm_getVar(source_string, RES_SENML, LEN_SENML);
  length = snprintf(buffer, REST_MAX_CHUNK_SIZE, source_string, "/led_d", "%", led_dim.value(SENSORS_ACTIVE));

  REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
  REST.set_response_payload(response, buffer, length);
}

RESOURCE(res_led_dim, "rt=\"gobi.a.light.dim\";if=\"core.a\"", led_dim_resource_handler, led_dim_resource_handler, led_dim_resource_handler, 0);
