#include "dtls_prf.h"

#include "dtls_aes.h"

#include <string.h>
/*---------------------------------------------------------------------------*/

#define DEBUG 0

#if DEBUG
    #include <stdio.h>
    #define PRINTF(...) printf(__VA_ARGS__)
#else
    #define PRINTF(...)
#endif

/* Private Funktionsprototypen --------------------------------------------- */

/* Öffentliche Funktionen -------------------------------------------------- */

void prf(uint8_t *dst, uint8_t len, uint8_t *data, size_t secret_len, size_t seed_len) {
    CMAC_State_t state;
    aes_cmac_init(&state, data, secret_len);

    // A(1) generieren
    uint8_t ax[16];
    aes_cmac_update(&state, data + secret_len, seed_len);
    aes_cmac_finish(&state, ax, 16);

    while (len > 0) {
        uint8_t result[16];
        aes_cmac_update(&state, ax, 16);
        aes_cmac_update(&state, data + secret_len, seed_len);
        aes_cmac_finish(&state, result, 16);
        memcpy(dst, result, len < 16 ? len : 16);

        // Falls weitere Daten benötigt werden, wird der Pointer und die
        // Länge entsprechend angepasst und ax weiterentwickelt
        if (len > 16) {
            dst += 16;
            len -= 16;

            uint8_t oldA[16];
            memcpy(oldA, ax, 16);
            aes_cmac_update(&state, oldA, 16);
            aes_cmac_finish(&state, ax, 16);
        } else {
            len = 0;
        }
    }
}

/* Private Funktionen ------------------------------------------------------ */

