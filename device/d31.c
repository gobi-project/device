#include "include/main_1.c"

// Nur benötigte device spezifische Dateien inkludieren
#include "include/r_dht.c"       // Temperatur-Sensor (i2c)

// Nur benötigte/eingebundene Sensoren registrieren
SENSORS(&button_sensor, &dht);

#include "include/main_2.c"

    // Nur benötigte Resourcen aktivieren ->
    rest_activate_resource(&res_dht_hum, "hum");
    rest_activate_resource(&res_dht_tmp, "tmp");

#include "include/main_3.c"

#include "include/main_4.c"

AUTOSTART_PROCESSES(&server_firmware);
