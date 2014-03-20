#include "device.h"
#include "flash-store.h"

#include "contiki.h"
#include "lib/sensors.h"
#include "i2c.h"


#ifndef Digital_Light_TSL2561_H
#define Digital_Light_TSL2561_H

#define  TSL2561_Control  0x80
#define  TSL2561_Timing   0x81 
#define  TSL2561_Interrupt 0x86
#define  TSL2561_DATA0LOW 0x8C
#define  TSL2561_DATA0HIGH 0x8D
#define  TSL2561_DATA1LOW 0x8E
#define  TSL2561_DATA1HIGH 0x8F

#define TSL2561_ADDR  0x29       //device address

#define LUX_SCALE 14           // scale by 2^14
#define RATIO_SCALE 9          // scale ratio by 2^9
#define CH_SCALE 10            // scale channel values by 2^10
#define CHSCALE_TINT0 0x7517   // 322/11 * 2^CH_SCALE
#define CHSCALE_TINT1 0x0fe7   // 322/81 * 2^CH_SCALE

#define K1T 0x0040   // 0.125 * 2^RATIO_SCALE
#define B1T 0x01f2   // 0.0304 * 2^LUX_SCALE
#define M1T 0x01be   // 0.0272 * 2^LUX_SCALE
#define K2T 0x0080   // 0.250 * 2^RATIO_SCA
#define B2T 0x0214   // 0.0325 * 2^LUX_SCALE
#define M2T 0x02d1   // 0.0440 * 2^LUX_SCALE
#define K3T 0x00c0   // 0.375 * 2^RATIO_SCALE
#define B3T 0x023f   // 0.0351 * 2^LUX_SCALE
#define M3T 0x037b   // 0.0544 * 2^LUX_SCALE
#define K4T 0x0100   // 0.50 * 2^RATIO_SCALE
#define B4T 0x0270   // 0.0381 * 2^LUX_SCALE
#define M4T 0x03fe   // 0.0624 * 2^LUX_SCALE
#define K5T 0x0138   // 0.61 * 2^RATIO_SCALE
#define B5T 0x016f   // 0.0224 * 2^LUX_SCALE
#define M5T 0x01fc   // 0.0310 * 2^LUX_SCALE
#define K6T 0x019a   // 0.80 * 2^RATIO_SCALE
#define B6T 0x00d2   // 0.0128 * 2^LUX_SCALE
#define M6T 0x00fb   // 0.0153 * 2^LUX_SCALE
#define K7T 0x029a   // 1.3 * 2^RATIO_SCALE
#define B7T 0x0018   // 0.00146 * 2^LUX_SCALE
#define M7T 0x0012   // 0.00112 * 2^LUX_SCALE
#define K8T 0x029a   // 1.3 * 2^RATIO_SCALE
#define B8T 0x0000   // 0.000 * 2^LUX_SCALE
#define M8T 0x0000   // 0.000 * 2^LUX_SCALE

#define K1C 0x0043   // 0.130 * 2^RATIO_SCALE
#define B1C 0x0204   // 0.0315 * 2^LUX_SCALE
#define M1C 0x01ad   // 0.0262 * 2^LUX_SCALE
#define K2C 0x0085   // 0.260 * 2^RATIO_SCALE
#define B2C 0x0228   // 0.0337 * 2^LUX_SCALE
#define M2C 0x02c1   // 0.0430 * 2^LUX_SCALE
#define K3C 0x00c8   // 0.390 * 2^RATIO_SCALE
#define B3C 0x0253   // 0.0363 * 2^LUX_SCALE
#define M3C 0x0363   // 0.0529 * 2^LUX_SCALE
#define K4C 0x010a   // 0.520 * 2^RATIO_SCALE
#define B4C 0x0282   // 0.0392 * 2^LUX_SCALE
#define M4C 0x03df   // 0.0605 * 2^LUX_SCALE
#define K5C 0x014d   // 0.65 * 2^RATIO_SCALE
#define B5C 0x0177   // 0.0229 * 2^LUX_SCALE
#define M5C 0x01dd   // 0.0291 * 2^LUX_SCALE
#define K6C 0x019a   // 0.80 * 2^RATIO_SCALE
#define B6C 0x0101   // 0.0157 * 2^LUX_SCALE
#define M6C 0x0127   // 0.0180 * 2^LUX_SCALE
#define K7C 0x029a   // 1.3 * 2^RATIO_SCALE
#define B7C 0x0037   // 0.00338 * 2^LUX_SCALE
#define M7C 0x002b   // 0.00260 * 2^LUX_SCALE
#define K8C 0x029a   // 1.3 * 2^RATIO_SCALE
#define B8C 0x0000   // 0.000 * 2^LUX_SCALE
#define M8C 0x0000   // 0.000 * 2^LUX_SCALE

#endif // Digital_Light_TSL2561_H


static void lux_init()
{
  uint8_t tx_buf[] = {
    TSL2561_Control, 0x03,  //power on
    TSL2561_Timing, 0x11,   //first: high gain mode, second: integration time 101ms
    TSL2561_Interrupt, 0x00 
  };

  i2c_transmitinit(TSL2561_ADDR, 2, &tx_buf[0]);
  while( !i2c_transferred() );

  i2c_transmitinit(TSL2561_ADDR, 2, &tx_buf[2]);
  while( !i2c_transferred() );

  i2c_transmitinit(TSL2561_ADDR, 2, &tx_buf[4]);
  while( !i2c_transferred() );


  PRINTF("finished init, check status \n");
  
  uint8_t results[1];
  uint8_t reg[1];
  reg[0] = 0x0A;  //REGISTER_ID
  i2c_transmitinit(TSL2561_ADDR, 1, reg);
  while (!i2c_transferred()); // wait for data to arrive
  i2c_receiveinit(TSL2561_ADDR, 1, results);
  while (!i2c_transferred()); // wait for data to arrive

  if(!(results[1] & 0x0A))
  {
    PRINTF("not inititialized, %02x\n", results[1]);
  }
  else
  {
    PRINTF("inititialized, %02x\n", results[1]);
  }
  
}

/* SENSOR ------------------------------------------------------------------ */

static int lux_value(int type) {
  int lux = 0;

  //read data from lux sensor
  uint8_t ch0_low = 0;
  uint8_t ch0_high = 0;
  uint8_t ch1_low = 0;
  uint8_t ch1_high = 0;

  
  uint8_t reg[1];
  uint8_t result[1];

  reg[0] = TSL2561_DATA0LOW;
  i2c_transmitinit(TSL2561_ADDR, 1, reg);
  while (!i2c_transferred()); // wait for data to arrive
  i2c_receiveinit(TSL2561_ADDR, 1, result);
  while (!i2c_transferred()); // wait for data to arrive
  ch0_low = result[0];

  reg[0] = TSL2561_DATA0HIGH;
  i2c_transmitinit(TSL2561_ADDR, 1, reg);
  while (!i2c_transferred()); // wait for data to arrive
  i2c_receiveinit(TSL2561_ADDR, 1, result);
  while (!i2c_transferred()); // wait for data to arrive
  ch0_high = result[0];

  reg[0] = TSL2561_DATA1LOW;
  i2c_transmitinit(TSL2561_ADDR, 1, reg);
  while (!i2c_transferred()); // wait for data to arrive
  i2c_receiveinit(TSL2561_ADDR, 1, result);
  while (!i2c_transferred()); // wait for data to arrive
  ch1_low = result[0];

  reg[0] = TSL2561_DATA1HIGH;
  i2c_transmitinit(TSL2561_ADDR, 1, reg);
  while (!i2c_transferred()); // wait for data to arrive
  i2c_receiveinit(TSL2561_ADDR, 1, result);
  while (!i2c_transferred()); // wait for data to arrive
  ch1_high = result[0];
  
  uint16_t channel0 = ch0_high * 256 + ch0_low;
  uint16_t channel1 = ch1_high * 256 + ch1_low;
  //finished reading
  
  //PRINTF( "c0l: %d\nc0h: %d\nc1l: %d\nc1h: %d", ch0_low, ch0_high, ch1_low, ch1_high );

  //config
  uint8_t tInt = 1;
  uint8_t iGain = 0;
  uint8_t iType = 1;

  int chscale = 0;
  int ratio1 = 0;
  int b = 0;
  int m = 0;

  //calculate lux
  switch(tInt)
  {
    case 0:  // 13.7 msec
        chscale = CHSCALE_TINT0;
      break;
    case 1: // 101 msec
        chscale = CHSCALE_TINT1;
      break;
    default: // assume no scaling
        chscale = (1 << CH_SCALE);
      break;
  }

  if(!iGain)
  {
    chscale = chscale << 4; // scale 1X to 16X
  }

  // scale the channel values
  channel0 = (channel0 * chscale) >> CH_SCALE;
  channel1 = (channel1 * chscale) >> CH_SCALE; 

  if(channel0!= 0)
  {
    ratio1 = (channel1 << (RATIO_SCALE+1)) / channel0;
  } 
  // round the ratio value
  unsigned long ratio = (ratio1 + 1) >> 1;
 
  switch (iType)
  {
    case 0: // T package
        if ((ratio >= 0) && (ratio <= K1T))
          {b=B1T; m=M1T;}
        else if (ratio <= K2T)
          {b=B2T; m=M2T;}
        else if (ratio <= K3T)
          {b=B3T; m=M3T;}
        else if (ratio <= K4T)
          {b=B4T; m=M4T;}
        else if (ratio <= K5T)
          {b=B5T; m=M5T;}
        else if (ratio <= K6T)
          {b=B6T; m=M6T;}
        else if (ratio <= K7T)
          {b=B7T; m=M7T;}
        else if (ratio > K8T)
          {b=B8T; m=M8T;}
      break;
    case 1:// CS package
        if ((ratio >= 0) && (ratio <= K1C))
          {b=B1C; m=M1C;}
        else if (ratio <= K2C)
          {b=B2C; m=M2C;}
        else if (ratio <= K3C)
          {b=B3C; m=M3C;}
        else if (ratio <= K4C)
          {b=B4C; m=M4C;}
        else if (ratio <= K5C)
          {b=B5C; m=M5C;}
        else if (ratio <= K6C)
          {b=B6C; m=M6C;}
        else if (ratio <= K7C)
           {b=B7C; m=M7C;}
       break;
  } 

  int temp = (( channel0 * b )-( channel1 * m ));
  if(temp < 0)
  {
    temp = 0;
  }

  temp += (1 << LUX_SCALE - 1);
  // strip off fractional portion
  lux = temp >> LUX_SCALE;

  return lux;
}

static int lux_status(int type) {
  switch (type) {
    case SENSORS_ACTIVE:
    case SENSORS_READY:
      return 1; // fix?
      break;
  }

  return 0;
}

static int lux_configure(int type, int c) {
  switch (type) {
    case SENSORS_HW_INIT:
      if (c) {
        //i2c_disable();

      } else {
        i2c_enable();
        lux_init();
      }
      return 1;
    default:
      return 0;
  }
}

SENSORS_SENSOR(lux, "Lux", lux_value, lux_configure, lux_status); // register the functions

/* RESOURCE ---------------------------------------------------------------- */

void lux_resource_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {
  int length = 0;

  uint8_t source_string[LEN_SENML_TMP]; // {"bn":"/tmp","bu":"%%degC","e":[{"v":"%d.%d"}]}
  nvm_getVar(source_string, RES_SENML_TMP, LEN_SENML_TMP);
  length = snprintf(buffer, REST_MAX_CHUNK_SIZE, source_string, lux.value(SENSORS_ACTIVE) / 100, lux.value(SENSORS_ACTIVE) % 100);

  REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
  REST.set_response_payload(response, buffer, length);
}

void lux_periodic_handler();

PERIODIC_RESOURCE(res_lux, "rt=\"gobi.s.tmp\";if=\"core.s\";obs", lux_resource_handler, NULL, NULL, NULL, 5 * CLOCK_SECOND, lux_periodic_handler);

void lux_periodic_handler() {
  REST.notify_subscribers(&res_lux);
}
