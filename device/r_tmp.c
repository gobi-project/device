/*
 * An interface to the TI TMP102 temperature sensor
 * 12 bit temperature reading, 0.5 deg. Celsius accuracy
 * -----------------------------------------------------------------
 *
 * Original Author  : Hedde Bosman (heddebosman@incas3.eu)
 */

#include "device.h"
#include "flash-store.h"

#include "contiki.h"
#include "lib/sensors.h"
#include "i2c.h"


#ifndef __TMP102_SENSOR_H
#define __TMP102_SENSOR_H

#define TMP102_VALUE_TYPE_DEFAULT 0

#define TMP102_ADDR                   0x48 //Addr0-Pin to Ground

#define TMP102_REGISTER_TEMPERATURE   0x00
#define TMP102_REGISTER_CONFIGURATION 0x01
#define TMP102_REGISTERO_T_LOW        0x02
#define TMP102_REGISTERO_T_HIGH       0x03

#define TMP102_CONF_EXTENDED_MODE     0x10
#define TMP102_CONF_ALERT             0x20
#define TMP102_CONF_CONVERSION_RATE   0xC0 // 2 bits indicating conversion rate (0.25, 1, 4, 8 Hz)

#define TMP102_CONF_SHUTDOWN_MODE     0x01
#define TMP102_CONF_THERMOSTAT_MODE   0x02 // 0 = comparator mode, 1 = interrupt mode
#define TMP102_CONF_POLARITY          0x04
#define TMP102_CONF_FAULT_QUEUE       0x18 // 2 bits indicating number of faults
#define TMP102_CONF_RESOLUTION        0x60 // 2 bits indicating resolution, default = b11 = 0x60
#define TMP102_CONF_ONESHOT_READY     0x80 //

#endif

static void set_configuration(uint8_t rate, uint8_t precision) {
  uint8_t tx_buf[] = {
    TMP102_REGISTER_CONFIGURATION,
    0,
    (precision ? TMP102_CONF_EXTENDED_MODE : 0) | ((rate << 6) & TMP102_CONF_CONVERSION_RATE)
  };

  i2c_transmitinit(TMP102_ADDR, 3, tx_buf);
}

/* SENSOR ------------------------------------------------------------------ */

static int tmp_value(int type) {
  uint8_t reg = TMP102_REGISTER_TEMPERATURE;
  uint8_t temp[2];
  int16_t temperature = 0;

  /* transmit the register to start reading from */
  i2c_transmitinit(TMP102_ADDR, 1, &reg);
  while (!i2c_transferred()); // wait for data to arrive

  /* receive the data */
  i2c_receiveinit(TMP102_ADDR, 2, temp);
  while (!i2c_transferred()); // wait for data to arrive

  // 12 bit normal mode
  temperature = ((temp[0] <<8) | (temp[1])) >> 4; // lsb

  // 13 bit extended mode
  //temperature = ((temp[0] <<8) | (temp[1])) >> 3; // lsb

  temperature = (100*temperature)/16; // in 100th of degrees

  return temperature;
}

static int tmp_status(int type) {
  switch (type) {
    case SENSORS_ACTIVE:
    case SENSORS_READY:
      return 1; // fix?
      break;
  }

  return 0;
}

static int tmp_configure(int type, int c) {
  switch (type) {
    case SENSORS_HW_INIT:
      if (c) {
        // set inactive
      } else {
        i2c_enable();
        set_configuration(1, 0); // every 1 second, 12bit precision
      }
      return 1;
    default:
      return 0;
  }
}

SENSORS_SENSOR(tmp, "Tmp", tmp_value, tmp_configure, tmp_status); // register the functions

/* RESOURCE ---------------------------------------------------------------- */

void tmp_resource_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {
  int length = 0;

  uint8_t source_string[LEN_SENML_TMP]; // {"bn":"/tmp","bu":"%%degC","e":[{"v":"%d.%d"}]}
  nvm_getVar(source_string, RES_SENML_TMP, LEN_SENML_TMP);
  length = snprintf(buffer, REST_MAX_CHUNK_SIZE, source_string, tmp.value(SENSORS_ACTIVE) / 100, tmp.value(SENSORS_ACTIVE) % 100);

  REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
  REST.set_response_payload(response, buffer, length);
}

void tmp_periodic_handler();

PERIODIC_RESOURCE(res_tmp, "rt=\"gobi.s.tmp\";if=\"core.s\";obs", tmp_resource_handler, NULL, NULL, NULL, 5 * CLOCK_SECOND, tmp_periodic_handler);

void tmp_periodic_handler() {
  REST.notify_subscribers(&res_tmp);
}
