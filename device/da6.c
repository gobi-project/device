#include "include/main_1.c"

// Nur benötigte device spezifische Dateien inkludieren
#include "include/r_b_out.c" 

// Nur benötigte/eingebundene Sensoren registrieren
SENSORS(&button_sensor, &b_out);

#include "include/main_2.c"

    // Nur benötigte Resourcen aktivieren ->
    rest_activate_resource(&res_b_out, "swt");
    
#include "include/main_3.c"

#include "include/main_4.c"

AUTOSTART_PROCESSES(&server_firmware);
