#include "device.h"
#include "button-sensor.h"
#include "leds.h"
#include "clock.h"
#include "er-coap-engine.h"
#include "flash-store.h"

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
  #define PRINT6ADDR(...)
#endif

// Sensoren und Resourcen einfügen - BEGIN
#include "r_device.c"
#include "r_flasher.c"
#include "r_led_bin.c"
#include "r_led_dim.c"
#include "r_tmp.c"
#include "r_button_ex.c"
// Sensoren und Resourcen einfügen - END

SENSORS(&button_sensor, &externbutton_sensor, &led_bin, &led_dim, &tmp);

// Start Process
PROCESS(server_firmware, "Server Firmware");
AUTOSTART_PROCESSES(&server_firmware);

PROCESS_THREAD(server_firmware, ev, data) {
  PROCESS_BEGIN();

  uint32_t time;
  nvm_getVar((void *) &time, RES_FLASHTIME, LEN_FLASHTIME);
  clock_set_seconds(time);

  leds_arch_init();
  leds_on(LEDS_GREEN);
    #if RADIODEBUGLED
      /* control TX_ON with the radio */
      GPIO->FUNC_SEL.GPIO_44 = 2;
      GPIO->PAD_DIR.GPIO_44 = 1;
    #endif
    nvm_init();
    // Resourcen aktivieren - BEGIN
    rest_init_engine();
    rest_activate_resource(&res_device, "d");
    rest_activate_resource(&res_time, "time");
    rest_activate_resource(&res_flasher, "f");
    rest_activate_resource(&res_led_bin, "led_b");
    rest_activate_resource(&res_led_dim, "led_d");
    rest_activate_resource(&res_tmp, "tmp");
    // Resourcen aktivieren - END
  leds_off(LEDS_GREEN);

  PRINTF("Firmware gestartet.\n");

  while(1) {
    PROCESS_WAIT_EVENT();
    
    if (ev == sensors_event) {
      if (data == &externbutton_sensor) {
        printf("extern button \n");
      }
      if (data == &button_sensor) {
        printf("board button \n");
        uip_ipaddr_t *addr = uip_ds6_defrt_choose();

        if (addr != 0) {
          PRINTF("Gateway: ");
          PRINT6ADDR(addr);
          PRINTF("\n");

          struct uip_udp_conn *udp_conn;
          uint8_t zaddr[16];
          uiplib_ipaddrconv ("aaaa::1",(uip_ipaddr_t *) &zaddr);
          udp_conn = uip_udp_new((uip_ipaddr_t *) &zaddr, UIP_HTONS(5684));
          uint8_t hello_request[3] = {0x50, 0x03, 0x00};
          uip_udp_packet_send(udp_conn, hello_request, 3);
          uip_udp_remove(udp_conn);
        } else {
          PRINTF("Gateway unbekannt!\n");
        }

        #if DEBUG
          PRINTF("\nFrei: %u Byte\n", 0x418000 - (uint32_t) &__heap_end__);

          // Folgende Ausgaben möglich durch Speicherinitialisierung in
          // contiki/platform/econotag/main.c durch hinzufügen der Flags
          // STACKMONITOR und HEAPMONITOR
          uint32_t p;
          p = (uint32_t) &__und_stack_top__;
          do {
            if (*(uint32_t *)p != 0x42424242) {
              PRINTF("Nie benutzer Stack > %d Byte\n", p - (uint32_t) &__und_stack_top__);
              break;
            }
            p += 16;
          } while (p < (uint32_t) &__sys_stack_top__ - 100);
          p = (uint32_t) &__heap_end__ - 4;
          do {
            if (*(uint32_t *)p != 0x42424242) {
              break;
            }
            p -= 4;
          } while (p >= (uint32_t) &__heap_start__);
          PRINTF("Nie benutzer Heap >= %d Byte\n", (uint32_t) &__heap_end__ - p - 4);

          PRINTF("\n");
        #endif
      }
    }
  }

  PROCESS_END();
}
