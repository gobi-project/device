#include "button-sensor.h"
#include "leds.h"
#include "clock.h"
#include "er-coap-engine.h"
#include "flash-store.h"
#include "storage.h"

#define DEBUG 1

#if DEBUG
  #include <stdio.h>
  #include "contiki-net.h"
  #define PRINTF(...) printf(__VA_ARGS__)

#define PRINT6ADDR(addr) PRINTF("[%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x]", ((uint8_t *)addr)[0], ((uint8_t *)addr)[1], ((uint8_t *)addr)[2], ((uint8_t *)addr)[3], ((uint8_t *)addr)[4], ((uint8_t *)addr)[5], ((uint8_t *)addr)[6], ((uint8_t *)addr)[7], ((uint8_t *)addr)[8], ((uint8_t *)addr)[9], ((uint8_t *)addr)[10], ((uint8_t *)addr)[11], ((uint8_t *)addr)[12], ((uint8_t *)addr)[13], ((uint8_t *)addr)[14], ((uint8_t *)addr)[15])

  extern uint32_t _start, _edata;
  extern uint32_t __stack_start__;
  extern uint32_t __irq_stack_top__;
  extern uint32_t __fiq_stack_top__;
  extern uint32_t __svc_stack_top__;
  extern uint32_t __abt_stack_top__;
  extern uint32_t __und_stack_top__;
  extern uint32_t __sys_stack_top__;
  extern uint32_t __bss_start__, __bss_end__;
  extern uint32_t __heap_start__, __heap_end__;
#else
  #define PRINTF(...)
#endif

// Sensoren und Resourcen einfügen - BEGIN
#include "attributes.c"
#include "flasher.c"
#include "led_bin.c"
 #include "led_dim.c"
// Sensoren und Resourcen einfügen - END

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
    // Sensoren aktivieren - BEGIN
    SENSORS_ACTIVATE(button_sensor);
    led_bin.configure(SENSORS_HW_INIT, 1);
    led_dim.configure(SENSORS_HW_INIT, 1);
    // Sensoren aktivieren - END
    // Resourcen aktivieren - BEGIN
    rest_init_engine();
    rest_activate_resource(&res_device, "d");
    rest_activate_resource(&res_time, "time");
    rest_activate_resource(&res_flasher, "f");
    rest_activate_resource(&res_led_bin, "led_b");
    rest_activate_resource(&res_led_dim, "led_d");
    // Resourcen aktivieren - END
  leds_off(LEDS_GREEN);

  PRINTF("Firmware gestartet.\n");

  while(1) {
    PROCESS_WAIT_EVENT();

    if (ev == sensors_event && data == &button_sensor) {
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
        PRINTF("\n");
        PRINTF("Speicheraufteilung (Konfiguration in contiki/cpu/mc1322x/mc1322x.lds)\n");
        PRINTF("---------------------------------------------------------------------\n");
        PRINTF("\n");
        PRINTF("Beschreibung | Start      | Ende       | Größe\n");
        PRINTF("----------------------------------------------\n");
        PRINTF("Programm     | 0x%08x | 0x%08x | %5u\n",
          &_start,            &_edata,            (uint32_t) &_edata            - (uint32_t) &_start);
        PRINTF("Irq Stack    | 0x%08x | 0x%08x | %5u\n",
          &__stack_start__,   &__irq_stack_top__, (uint32_t) &__irq_stack_top__ - (uint32_t) &__stack_start__);
        PRINTF("Fiq Stack    | 0x%08x | 0x%08x | %5u\n",
          &__irq_stack_top__, &__fiq_stack_top__, (uint32_t) &__fiq_stack_top__ - (uint32_t) &__irq_stack_top__);
        PRINTF("Svc Stack    | 0x%08x | 0x%08x | %5u\n",
          &__fiq_stack_top__, &__svc_stack_top__, (uint32_t) &__svc_stack_top__ - (uint32_t) &__fiq_stack_top__);
        PRINTF("Abt Stack    | 0x%08x | 0x%08x | %5u\n",
          &__svc_stack_top__, &__abt_stack_top__, (uint32_t) &__abt_stack_top__ - (uint32_t) &__svc_stack_top__);
        PRINTF("Und Stack    | 0x%08x | 0x%08x | %5u\n",
          &__abt_stack_top__, &__und_stack_top__, (uint32_t) &__und_stack_top__ - (uint32_t) &__abt_stack_top__);
        PRINTF("Sys Stack    | 0x%08x | 0x%08x | %5u\n",
          &__und_stack_top__, &__sys_stack_top__, (uint32_t) &__sys_stack_top__ - (uint32_t) &__und_stack_top__);
        PRINTF("Datensegment | 0x%08x | 0x%08x | %5u\n",
          &__bss_start__,     &__bss_end__,       (uint32_t) &__bss_end__       - (uint32_t) &__bss_start__);
        PRINTF("Heap         | 0x%08x | 0x%08x | %5u\n",
          &__heap_start__,    &__heap_end__,      (uint32_t) &__heap_end__      - (uint32_t) &__heap_start__);
        PRINTF("Frei         | 0x%08x | 0x%08x | %5u\n",
          &__heap_end__,      0x418000,           0x418000                      - (uint32_t) &__heap_end__);
        PRINTF("----------------------------------------------\n");
        PRINTF("Frei += 1108 bei der Deaktivierung dieser Auskunft\n");

        PRINTF("\n");

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

  PROCESS_END();
}
