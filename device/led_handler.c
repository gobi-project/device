#include "led_handler.h"
#include <rest-engine.h>
#include <string.h>

#ifndef min
#define min(x,y) ((x) < (y) ? (x) : (y))
#endif
/*---------------------------------------------------------------------------*/

static int led_status = 0;

static int
value(int type)
{
  switch(type)
  {
    case SENSORS_ACTIVE:
      return led_status;
      break;
    case SENSORS_READY:
      return led_status;
      break;
    default:
      break;
  }
  return 0; // error
}

static int
configure(int type, int c)
{
  if( type == SENSORS_ACTIVE )
  {
    gpio_set_pad_dir(42,1);

    if( 1 == c && 0 == led_status ) 
    {
      gpio_set(42);
      led_status = 1;
    }
    else 
    {
      gpio_reset(42);
      led_status = 0;
    }

    return 1; // no error
  }

  return 0; // error
}

static int
status(int type)
{
  switch (type)
  {
    case SENSORS_ACTIVE:
      return 1; // no error
      break;
    case SENSORS_READY:
      return 1; // no error
      break;
  }

  return 0; // error
}

SENSORS_SENSOR(led, "LED", value, configure, status); // register the functions

RESOURCE(res_led, "rt=\"led\";if=\"core.s\"", led_resource_handler, led_resource_handler, led_resource_handler, 0 );

void
led_resource_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  char payload[32];
  int length = 0;
  buffer[REST_MAX_CHUNK_SIZE-1] = 0;

  if( METHOD_GET == REST.get_method_type(request) ) 
  {
    //get current sensor state
    int sensor_status = led.value( SENSORS_ACTIVE );
   
    //muss da sein????
    // const char *payload_query = 0;

    // if(request) 
    // {
    //   
    //   REST.get_query_variable(request, "input", &payload_query);
    //   
    //   input = atoi(payload_query);
    //   
    //   
    // }

    snprintf( payload, 32, "%d", sensor_status );

    char cent[3];
    sprintf(cent, "%s", payload+strlen(payload)-2);
    payload[strlen(payload)-2] = 0;

    char gigaload[128];
    snprintf(gigaload, 128, "{\"bn\":\"/r/10\",\"bu\":\"nil\",\"e\":[{\"v\":\"%s.%s\"}]}",payload, cent);
    
    memcpy(buffer,gigaload,length = min(strlen(gigaload), REST_MAX_CHUNK_SIZE-1));    
  }
  else 
  {
    const uint8_t *req_payload = 0;
    int len = 0;
    len = REST.get_request_payload(request, &req_payload);
    memset(payload, 0, 32);
    memcpy(payload, req_payload, min(31, len));

    //set sensor state
    int result = led.configure( SENSORS_ACTIVE, atoi(payload) );
    snprintf(payload,32,"%d",result); // result = 0: error, result = 1: no error

    memcpy(buffer,payload,length = min(strlen(payload), REST_MAX_CHUNK_SIZE-1));
  }

  REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
  REST.set_header_etag(response, (uint8_t *) &length, 1);
  REST.set_response_payload(response, buffer, length);
}

void
led_init_handler()
{
  SENSORS_ACTIVATE(led);
}