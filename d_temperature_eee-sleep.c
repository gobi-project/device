#include "include/main_1_eee.c"

// Nur benötigte device spezifische Dateien inkludieren
#include "include/r_tmp.c"       // Temperatur-Sensor (i2c)

// Nur benötigte/eingebundene Sensoren registrieren
SENSORS(&button_sensor, &tmp);

#include "include/main_2.c"

    // Nur benötigte Resourcen aktivieren ->
    rest_activate_resource(&res_tmp, "tmp");

#include "include/main_3_eee-sleep.c"

#include "include/main_4_eee-sleep.c"

AUTOSTART_PROCESSES(&server_firmware);