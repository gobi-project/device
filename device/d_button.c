#include "include/main_1.c"

// Nur benötigte device spezifische Dateien inkludieren
#include "include/r_button_ex.c"       // Temperatur-Sensor (i2c)

// Nur benötigte/eingebundene Sensoren registrieren
SENSORS(&button_sensor, &externbutton_sensor);

#include "include/main_2.c"

    // Nur benötigte Resourcen aktivieren ->
    rest_activate_resource(&res_btn, "btn");
    
#include "include/main_3.c"

#include "include/main_4.c"

PROCESS(event_listener, "Event Listener");
PROCESS_THREAD(event_listener, ev, data) {
  PROCESS_BEGIN();

  while(1) {
    PROCESS_WAIT_EVENT();

    if (ev == sensors_event) {
      if (data == &externbutton_sensor) {
        res_btn.trigger();
        PRINTF("extern button\n");
      }
    }
  }
  PROCESS_END();
}

// server_firmware muss immer gestartet werden. Weitere Prozesse, wie der
// event_listener in diesem Beispiel, müssen zusätzlich eingefügt werden.
AUTOSTART_PROCESSES(&server_firmware, &event_listener);
