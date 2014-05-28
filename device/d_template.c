#include "include/main_1.c"

// Nur benötigte device spezifische Dateien inkludieren
#include "include/r_button_ex.c" // externer Button (kbi6)
#include "include/r_led_bin.c"   // binäre LED (GPIO 43)
#include "include/r_led_dim.c"   // dimmbare LED (TMR2)
#include "include/r_tmp.c"       // Temperatur-Sensor (i2c)
#include "include/r_lux.c"       // Lux-Sensor (i2c)
#include "include/r_dht.c"       // digitaler Luftfeuchtigkeit- und Temperatur-Sensor (GPIO 42)
#include "include/r_poti.c"      // Potentiometer (ADC0)

// Nur benötigte/eingebundene Sensoren registrieren
SENSORS(&button_sensor, &button_sensor2, &externbutton_sensor, &led_bin, &led_dim, &tmp, &lux, &dht, &poti);

#include "include/main_2.c"

    // Nur benötigte Resourcen aktivieren ->
    rest_activate_resource(&res_btn, "btn");
    rest_activate_resource(&res_led_bin, "led_b");
    rest_activate_resource(&res_led_dim, "led_d");
    rest_activate_resource(&res_tmp, "tmp");
    rest_activate_resource(&res_lux, "lux");
    rest_activate_resource(&res_dht_hum, "hum");
    rest_activate_resource(&res_dht_tmp, "tmp");
    rest_activate_resource(&res_poti, "poti");

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
      }
      if (data == &button_sensor2) {
        printf("board button 2\n");
      }
      if (data == &externbutton_sensor) {
        res_btn.trigger();
        printf("extern button\n");
      }
    }
  }
  PROCESS_END();
}

// server_firmware muss immer gestartet werden. Weitere Prozesse, wie der
// event_listener in diesem Beispiel, müssen zusätzlich eingefügt werden.
AUTOSTART_PROCESSES(&server_firmware, &event_listener);
