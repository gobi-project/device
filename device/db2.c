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
