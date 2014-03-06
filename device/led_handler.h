#ifndef LED_HANDLER_H
#define LED_HANDLER_H

#include <lib/sensors.h>
#include <gpio-util.h>
#include <rest-engine.h>

extern const struct sensors_sensor led;

extern void
led_resource_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

extern void
led_init_handler();

extern resource_t*
get_led_resource();

#endif