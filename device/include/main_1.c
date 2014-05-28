#include "leds.h"
#include "button-sensor.h"
#include "clock.h"
#include "er-coap-engine.h"
#include "flash.h"
#include "../../../contiki/tools/blaster/blaster.h"

#include "contiki-net.h"

#define DEBUG 1

#if DEBUG
  #include <stdio.h>
  #define PRINTF(...) printf(__VA_ARGS__)
  #define PRINT6ADDR(addr) PRINTF("[%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x]", ((uint8_t *)addr)[0], ((uint8_t *)addr)[1], ((uint8_t *)addr)[2], ((uint8_t *)addr)[3], ((uint8_t *)addr)[4], ((uint8_t *)addr)[5], ((uint8_t *)addr)[6], ((uint8_t *)addr)[7], ((uint8_t *)addr)[8], ((uint8_t *)addr)[9], ((uint8_t *)addr)[10], ((uint8_t *)addr)[11], ((uint8_t *)addr)[12], ((uint8_t *)addr)[13], ((uint8_t *)addr)[14], ((uint8_t *)addr)[15])
  extern uint32_t __und_stack_top__;
  extern uint32_t __sys_stack_top__;
  extern uint32_t __heap_start__, __heap_end__;
#else
  #define PRINTF(...)
  #define PRINT6ADDR(addr)
#endif

static uint32_t send_hello = 1;

// Standard Ressourcen einfügen
#include "r_device.c"
#include "r_flasher.c"
