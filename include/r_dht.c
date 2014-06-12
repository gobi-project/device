#include "flash.h"
#include "../../../contiki/tools/blaster/blaster.h"

#include <lib/sensors.h>
#include <gpio-util.h>
#include <clock.h>
#include <isr.h>

#include <rest-engine.h>
#include <string.h>

#define DHT_PIN  42
#define DHT_HUMIDITY 10
#define DHT_TEMPERATURE 11


// DHT - digital humidity temperature
/* SENSOR ------------------------------------------------------------------ */

static uint16_t dht_tmp = 0;
static uint16_t dht_hum = 0;
static unsigned long dht_last_call = 0;

static void
dht_read()
{
  if( dht_last_call + 5 < clock_seconds() )
  {
    dht_last_call = clock_seconds();
  }
  else
  {
    return;
  }

  uint8_t data[5];
  uint8_t i = 0;
  for(i = 0; i < 5; i++)
  {
    data[i] = 0;
  }

  // pull the pin high and wait 250 milliseconds
  gpio_set_pad_dir( DHT_PIN, PAD_DIR_OUTPUT );
  gpio_set( DHT_PIN );
  clock_wait( CLOCK_SECOND / 4 );

  global_irq_disable(); //this is necessary to make sure that clock_delay_usec works properly
  // now pull it low for ~20 milliseconds
  gpio_reset( DHT_PIN );

  clock_delay_usec( 20000 );
  
  gpio_set( DHT_PIN );
  clock_delay_usec( 30 );
  gpio_reset( DHT_PIN );

  gpio_set_pad_dir( DHT_PIN, PAD_DIR_INPUT );

  //dht return 80 us low and 80 us high for initialisation
  while( !gpio_read( DHT_PIN ) ); //80 us low
  while( gpio_read( DHT_PIN ) ); //80 us high

  // read in timings
  uint8_t result = 0;
  uint8_t j = 0;
  for( i = 0; i < 5; i++ )
  {
    for( j = 0; j < 8; j++ )
    {
      while( !gpio_read( DHT_PIN ) ); //after this the signal is high
      clock_delay_usec( 50 );
      result = gpio_read( DHT_PIN );
      if( result )
      {
        //bit is high
        data[i] = data[i] << 1 | 1;
        while( gpio_read( DHT_PIN ) ); //wait for the next bit
      }
      else
      {
        //bit is low
        data[i] = data[i] << 1 | 0;
      }
    }
  }

  global_irq_enable();

  if( data[0] + data[1] + data[2] + data[3] != data[4] )
  {
    //incorrect checksum
    return;
  }

  dht_hum = data[0] * 256 + data[1];
  dht_tmp = data[2] * 256 + data[3];
}

static int dht_value(int type) 
{
  switch( type )
  {
    case DHT_HUMIDITY:
        return dht_hum;
      break;
    case DHT_TEMPERATURE:
        return ((dht_tmp - 3200) * 5 ) / 9;
      break;
    default:
        return 0;
      break;
  }
}

static int dht_configure(int type, int c) 
{
  switch (type) {
    case SENSORS_HW_INIT:
      gpio_set_pad_dir( DHT_PIN, PAD_DIR_OUTPUT );
      gpio_set( DHT_PIN );
      return 1;
    default:
      return 0;
  }
}

static int dht_status(int type) 
{
  if (type == SENSORS_HW_INIT) return 0;
  return 1; //todo
}

SENSORS_SENSOR(dht, "dht", dht_value, dht_configure, dht_status);

/* RESOURCE ---------------------------------------------------------------- */

void dht_hum_resource_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) 
{
  dht_read();
  uint16_t hum = dht.value(DHT_HUMIDITY);

  int length = 0;
  
  uint8_t source_string[LEN_SENML_HUM]; // {"bn":"/hum","bu":"%%RH","e":[{"v":"%d.%d"}]}
  flash_getVar(source_string, RES_SENML_HUM, LEN_SENML_HUM);
  length = snprintf(buffer, REST_MAX_CHUNK_SIZE, source_string, hum / 100, hum % 100);

  REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
  REST.set_response_payload(response, buffer, length);
}

void dht_tmp_resource_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) 
{
  dht_read();
  uint16_t tmp = dht.value(DHT_TEMPERATURE);

  int length = 0;

  uint8_t source_string[LEN_SENML_TMP_F]; // {"bn":"/tmp","bu":"%%degF","e":[{"v":"%d.%d"}]}
  flash_getVar(source_string, RES_SENML_TMP_F, LEN_SENML_TMP_F);
  length = snprintf(buffer, REST_MAX_CHUNK_SIZE, source_string, tmp / 100, tmp % 100);

  REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
  REST.set_response_payload(response, buffer, length);
}

void dht_hum_periodic_handler();
void dht_tmp_periodic_handler();

PERIODIC_RESOURCE(res_dht_hum, "rt=\"gobi.s.hum\";if=\"core.s\";obs", dht_hum_resource_handler, NULL, NULL, NULL, 5 * CLOCK_SECOND, dht_hum_periodic_handler);
PERIODIC_RESOURCE(res_dht_tmp, "rt=\"gobi.s.tmp\";if=\"core.s\";obs", dht_tmp_resource_handler, NULL, NULL, NULL, 5 * CLOCK_SECOND, dht_tmp_periodic_handler);

void dht_hum_periodic_handler() 
{
  REST.notify_subscribers(&res_dht_hum);
}

void dht_tmp_periodic_handler() 
{
  REST.notify_subscribers(&res_dht_tmp);
}
