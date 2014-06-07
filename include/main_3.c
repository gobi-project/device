  leds_off(LEDS_GREEN);

  PRINTF("Firmware gestartet.\n");

  while(1) {
    static struct etimer hello_timer;
    etimer_set(&hello_timer, CLOCK_SECOND * 30);

#if HELLO_REQUEST
    if (send_hello) {
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
    }
#endif
