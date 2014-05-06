#include "include/main_1.c"

// Nur benötigte device spezifische Dateien inkludieren

// Nur benötigte/eingebundene Sensoren registrieren
SENSORS(&button_sensor, &button_sensor2);

#include "include/main_2.c"

#include "include/main_3.c"

      if (data == &button_sensor2) {
        printf("board button 2\n");
      }

#include "include/main_4.c"
