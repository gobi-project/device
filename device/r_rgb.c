#include "device.h"
#include "flash-store.h"

#include <lib/sensors.h>
#include <gpio-util.h>
#include <clock.h>
#include <isr.h>

#include <rest-engine.h>
#include <string.h>

#define RGB_DMX_PIN  42

// DHT - digital humidity temperature
/* SENSOR ------------------------------------------------------------------ */

static int rgb_last_value = 0;

static void
rgb_dmx_output(int rgb)
{
  uint8_t data[5];
  uint16_t i = 0;
  for( i = 0; i < 5; i++ )
  {
    data[i] = 0;
  }
  
  //data[0]: start byte
  //data[1]: unused channel 1
  //data[2]: red
  //data[3]: green
  //data[4]: blue

  data[4] = rgb & 0xFF;   //get blue value
  rgb = rgb >> 8;
  data[3] = rgb & 0xFF;   //get green value
  rgb = rgb >> 8;
  data[2] = rgb & 0xFF;   //get red value

  global_irq_disable();

  gpio_set( RGB_DMX_PIN );
  clock_delay_usec( 10000 );  //MARK for 10 ms
  gpio_reset( RGB_DMX_PIN );
  clock_delay_usec( 125 );    //RESET
  gpio_set( RGB_DMX_PIN );
  clock_delay_usec( 8 );      //MARK
  gpio_reset( RGB_DMX_PIN );
  
  uint8_t current_value = 0;
  uint8_t j = 0;

  for( i = 0; i < 5; i++ )
  {
    //send start bit
    gpio_set( RGB_DMX_PIN );
    clock_delay_usec( 4 );
    gpio_reset( RGB_DMX_PIN );

    current_value = data[i];
    for( j = 0; j < 8; j++ )
    {
      //start with LSB
      if( current_value & 0x01 )
      {
        //high bit
        gpio_set( RGB_DMX_PIN );
      }
      else
      
      clock_delay_usec( 4 );
      gpio_reset( RGB_DMX_PIN );

      current_value = current_value >> 1;
    }

    //send stop bits
    gpio_set( RGB_DMX_PIN );
    clock_delay_usec( 4 );
    gpio_reset( RGB_DMX_PIN );

    gpio_set( RGB_DMX_PIN );
    clock_delay_usec( 4 );
    gpio_reset( RGB_DMX_PIN );
  }
  
  gpio_set( RGB_DMX_PIN );
  clock_delay_usec( 10000 );  //MARK for 10 ms
  gpio_reset( RGB_DMX_PIN );

  global_irq_enable();
  rgb_last_value = rgb;
}

static int rgb_value(int type) 
{
  switch( type )
  {
    case SENSORS_ACTIVE:
        return rgb_last_value;
      break;
    default:
        return 0;
      break;
  }
}

static int rgb_configure(int type, int c) 
{
  switch (type) {
    case SENSORS_HW_INIT:
      if(!c)
      {
        gpio_set_pad_dir( RGB_DMX_PIN, PAD_DIR_OUTPUT );
        gpio_reset( RGB_DMX_PIN );
        return 1;
      }
    case SENSORS_ACTIVE:
      rgb_dmx_output(c);
      return 1;
    default:
      return 0;
  }
}

static int rgb_status(int type) 
{
  if (type == SENSORS_HW_INIT) return 0;
  return 1; //todo
}

SENSORS_SENSOR(rgb, "rgb", rgb_value, rgb_configure, rgb_status);

/* RESOURCE ---------------------------------------------------------------- */

void rgb_resource_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {
  int length = 0;

  if (REST.get_method_type(request) != METHOD_GET) {
    const uint8_t *payload = 0;
    int len = 0;
    len = REST.get_request_payload(request, &payload);
    rgb.configure(SENSORS_ACTIVE, atoi(payload));
  }

  //TODO!!

  uint8_t source_string[LEN_SENML_LEDB]; // {"bn":"/led_b","bu":"B","e":[{"v":"%d"}]}
  nvm_getVar(source_string, RES_SENML_LEDB, LEN_SENML_LEDB);
  length = snprintf(buffer, REST_MAX_CHUNK_SIZE, source_string, rgb.value(SENSORS_ACTIVE));

  REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
  REST.set_response_payload(response, buffer, length);
}

RESOURCE(res_rgb, "rt=\"gobi.a.light.rgb\";if=\"core.a\"", rgb_resource_handler, rgb_resource_handler, rgb_resource_handler, 0);