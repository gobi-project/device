#include "include/main_1.c"

// Nur benötigte device spezifische Dateien inkludieren

// Nur benötigte/eingebundene Sensoren registrieren
SENSORS(&button_sensor, &button_sensor2);

#include "include/main_2.c"

    // Nur benötigte Resourcen aktivieren ->

#include "include/main_3.c"

  // Was nun folgt wird alle 30 Sekunden wiederholt ->
  printf("Hallo Welt!\n");

#include "include/main_4.c"

PROCESS(event_listener, "Event Listener");
PROCESS_THREAD(event_listener, ev, data) {
  PROCESS_BEGIN();

  while(1) {
    PROCESS_WAIT_EVENT();

    if (ev == sensors_event) {
      if (data == &button_sensor) {
        printf("board button 1\n");
#if DEBUG
        PRINTF("\nFrei: %u Byte\n", 0x418000 - (uint32_t) &__heap_end__);

        // Folgende Ausgaben möglich durch Speicherinitialisierung in
        // contiki/platform/econotag/main.c durch hinzufügen der Flags
        // STACKMONITOR und HEAPMONITOR
        uint32_t p;
#if STACKMONITOR
        p = (uint32_t) &__und_stack_top__;
        do {
          if (*(uint32_t *)p != 0x42424242) {
            PRINTF("Nie benutzer Stack > %d Byte\n", p - (uint32_t) &__und_stack_top__);
            break;
          }
          p += 16;
        } while (p < (uint32_t) &__sys_stack_top__ - 100);
#endif
#if HEAPMONITOR
        p = (uint32_t) &__heap_end__ - 4;
        do {
          if (*(uint32_t *)p != 0x42424242) {
            break;
          }
          p -= 4;
        } while (p >= (uint32_t) &__heap_start__);
        PRINTF("Nie benutzer Heap >= %d Byte\n", (uint32_t) &__heap_end__ - p - 4);
#endif
        PRINTF("\n");
#endif
      }
      if (data == &button_sensor2) {
        printf("board button 2\n");
      }
    }
  }
  PROCESS_END();
}

// server_firmware muss immer gestartet werden. Weitere Prozesse, wie der
// event_listener in diesem Beispiel, müssen zusätzlich eingefügt werden.
AUTOSTART_PROCESSES(&server_firmware, &event_listener);
