// Start Process
PROCESS(server_firmware, "Server Firmware");
AUTOSTART_PROCESSES(&server_firmware);

PROCESS_THREAD(server_firmware, ev, data) {
  PROCESS_BEGIN();

  uint32_t time;
  flash_getVar((void *) &time, RES_FLASHTIME, LEN_FLASHTIME);
  clock_set_seconds(time);

  leds_arch_init();
  leds_on(LEDS_GREEN);
#if RADIODEBUGLED
      /* control TX_ON with the radio */
      GPIO->FUNC_SEL.GPIO_44 = 2;
      GPIO->PAD_DIR.GPIO_44 = 1;
#endif
    flash_init();
    rest_init_engine();
    // Standard Ressourcen aktivieren
    rest_activate_resource(&res_device, "d");
    rest_activate_resource(&res_time, "time");
    rest_activate_resource(&res_flasher, "f");
