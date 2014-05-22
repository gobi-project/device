#include "storage.h"
#include "flash-store.h"

#include "contiki.h"
#include "lib/sensors.h"
#include "i2c.h"

#ifndef RGB_ACTUATOR
#define RGB_ACTUATOR
#define RGB_ADDR 0x04
#endif // RGB_ACTUATOR

/* SENSOR ------------------------------------------------------------------ */
static int current_rgb_value;

static void rgb_int2bytes(uint8_t* output, int rgb)
{
  output[2] = rgb & 0xFF;   //get blue value
  rgb = rgb >> 8;
  output[1] = rgb & 0xFF;   //get green value
  rgb = rgb >> 8;
  output[0] = rgb & 0xFF;   //get red value
}

static unsigned int rgb_bytes2int(uint8_t *input)
{
  unsigned int result = 0;
  result |= input[0];   //red comes first
  result = result << 8;
  result |= input[1];   //gren
  result = result << 8;
  result |= input[2];   //blue
  return result;
}

static int rgb_transmit(int rgb)
{
  current_rgb_value = rgb;

  uint8_t data[3];
  uint8_t i;
  for( i = 0; i < 3; i++ )
  {
    data[i] = 0;
  }

  rgb_int2bytes(data, rgb);

  //send rgb data
  i2c_transmitinit(RGB_ADDR, 3, data);
  while(!i2c_transferred());
  
  return 1;
}

static int rgb_value(int type) {
  if( SENSORS_ACTIVE == type )
  { 
    uint8_t data[3];
    i2c_receiveinit(RGB_ADDR, 3, data);
    while(!i2c_transferred());
    return rgb_bytes2int(data);
  }
  else
  {
    return 1;
  }
}

static int rgb_status(int type) {
  return 0;
}

static int rgb_configure(int type, int c) {
  switch (type) {
    case SENSORS_HW_INIT:
      if(!c)
      {
        i2c_enable();
        return 1;
      }
      else
      {
        i2c_disable();
        return 0;
      }
    case SENSORS_ACTIVE:
      return rgb_transmit(c);
    default:
      return 0;
  }
}

SENSORS_SENSOR(rgb, "RGB", rgb_value, rgb_configure, rgb_status); // register the functions

/* RESOURCE ---------------------------------------------------------------- */

void rgb_resource_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {
  int length = 0;
  int result = 0;

  if (REST.get_method_type(request) != METHOD_GET && *offset == 0) {
    const uint8_t *payload = 0;
    int len = 0;
    len = REST.get_request_payload(request, &payload);
    result = atoi(payload);
    rgb.configure(SENSORS_ACTIVE, result);
    clock_wait( CLOCK_SECOND >> 2 );
  }
  
  uint8_t source_string[LEN_SENML_RGB];  // {"bn":"/rgb","bu":"ARGB","e":[{"v":"%u"}]}
  nvm_getVar(source_string, RES_SENML_RGB, LEN_SENML_RGB);
  length = snprintf(buffer, REST_MAX_CHUNK_SIZE, source_string, rgb.value(SENSORS_ACTIVE) );

  REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
  REST.set_response_payload(response, buffer, length);
}

RESOURCE(res_rgb, "rt=\"gobi.a.light.rgb\";if=\"core.a\"", rgb_resource_handler, rgb_resource_handler, rgb_resource_handler, 0);
