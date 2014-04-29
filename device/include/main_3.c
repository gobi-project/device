  leds_off(LEDS_GREEN);

  PRINTF("Firmware gestartet.\n");

  while(1) {
    PROCESS_WAIT_EVENT();

    if (ev == sensors_event) {
      if (data == &button_sensor) {
        printf("board button 1\n");
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
