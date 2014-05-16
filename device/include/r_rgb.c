#include "storage.h"
#include "flash-store.h"

#include "contiki.h"
#include "lib/sensors.h"
#include "i2c.h"

#ifndef RGB_ACTUATOR
#define RGB_ACTUATOR
#define RGB_ADDR 0x04
#define RGB_IDLE 0xF1
#define RGB_TEST 0xF2
#define RGB_TRAN 0xF3
#endif // RGB_ACTUATOR

/* SENSOR ------------------------------------------------------------------ */
static int current_rgb_value;

static void rgb_int2bytes(char* output, int rgb)
{
  output[2] = rgb & 0xFF;   //get blue value
  rgb = rgb >> 8;
  output[1] = rgb & 0xFF;   //get green value
  rgb = rgb >> 8;
  output[0] = rgb & 0xFF;   //get red value
}

static int rgb_bytes2int(char *input)
{
  int result = 0;
  result = input[2];
  result = result << 8;
  result = result & input[1];
  result = result << 8;
  result = result & input[0];
  return result;
}

static int rgb_run_test()
{
  uint8_t mode = RGB_TEST;
  i2c_transmitinit(RGB_ADDR, 1, &mode);
  while(!i2c_transferred());

  clock_wait( CLOCK_SECOND * 5 );   //wait until the test is done

  i2c_receiveinit(RGB_ADDR, 1, &mode);   //return RGB_IDLE if everything is correct
  while (!i2c_transferred());

  return ( mode == RGB_TRAN );
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
      if(!c)
      {
        i2c_enable();
        clock_wait( CLOCK_SECOND * 10 ); //wait until the arduino is initialized
        return rgb_run_test();
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

  if (REST.get_method_type(request) != METHOD_GET) {
    const uint8_t *payload = 0;
    int len = 0;
    len = REST.get_request_payload(request, &payload);
    result = atoi(payload);
    rgb.configure(SENSORS_ACTIVE, result);
  }
  
  uint8_t rgb_values[3];
  rgb_int2bytes(rgb_values, rgb_value(SENSORS_ACTIVE));

  // {"bn":"/rgb","e":[{"n":"red","v":%d,"u":"%"},{"n":"green","v":%d,"u":"%"},{"n":"blue","v":%d,"u":"%"}]} 
  uint8_t source_string[LEN_SENML_RGB];
  nvm_getVar(source_string, RES_SENML_RGB, LEN_SENML_RGB);
  length = snprintf(buffer, REST_MAX_CHUNK_SIZE, source_string, rgb_values[2], rgb_values[1], rgb_values[0]);

  REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
  REST.set_response_payload(response, buffer, length);
}

RESOURCE(res_rgb, "rt=\"gobi.a.light.rgb\";if=\"core.a\"", rgb_resource_handler, rgb_resource_handler, rgb_resource_handler, 0);