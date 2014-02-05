#include "dtls_aes.h"

#include <string.h>
#include <arpa/inet.h>
#include <openssl/evp.h>

/*---------------------------------------------------------------------------*/

#define DEBUG 0

#if DEBUG
    #include <stdio.h>
    #define PRINTF(...) printf(__VA_ARGS__)
    void print_hex(uint8_t *d, uint8_t l) {
        int i;
        for (i = 0; i < l; i++) printf("%02X", d[i]);
    }
#else
    #define PRINTF(...)
#endif

#define min(x,y) ((x)<(y)?(x):(y))

/* Private Funktionsprototypen --------------------------------------------- */

void getBlockKey(uint8_t *out, uint8_t *key, uint8_t *nonce, uint32_t index);
void setAuthCode(uint8_t data[], size_t data_len, uint8_t key[16], uint8_t nonce[NONCE_LEN]);
void crypt(uint8_t data[], size_t data_len, uint8_t key[16], uint8_t nonce[NONCE_LEN]);
__attribute__((always_inline)) static void cmac_subkey(uint8_t L[16], uint8_t K);

/* Öffentliche Funktionen -------------------------------------------------- */

void aes_encrypt(uint8_t data[], size_t data_len, uint8_t key[16], uint8_t nonce[NONCE_LEN]) {
    setAuthCode(data, data_len, key, nonce);
    crypt(data, data_len, key, nonce);
}

void aes_decrypt(uint8_t data[], size_t data_len, uint8_t key[16], uint8_t nonce[NONCE_LEN]) {
    crypt(data, data_len, key, nonce);
    setAuthCode(data, data_len, key, nonce);
}

void aes_cmac(uint8_t mac[16], uint8_t data[], size_t data_len, uint8_t key[16], uint8_t finish) {
    uint32_t i;

    if (data_len == 0) {
        printf("aes_cmac: Ungültiger Aufruf. data_len == 0 ist nicht zulässig.\n");
        return;
    }
    if (!finish && data_len % 16) {
        printf("aes_cmac: Ungütiger Aufruf. Bei finish == 0 muss data_len ein Vielfaches der Blockgröße sein.\n");
        return;
    }

    #if DEBUG
        printf("Key      ");
        print_hex(key, 16);
        printf("\n");
    #endif

    EVP_CIPHER_CTX ctx;
    EVP_CIPHER_CTX_init(&ctx);
    EVP_EncryptInit(&ctx, EVP_aes_128_cbc(), key, NULL);

    uint8_t buf[16], result[16];
    int32_t cypherLen;

    if (finish) {
        memset(buf, 0, 16);
        EVP_EncryptUpdate(&ctx, result, &cypherLen, buf, 16);
    }
    #if DEBUG
        printf("K0       ");
        print_hex(result, 16);
        printf("\n");
    #endif

    EVP_CIPHER_CTX_cleanup(&ctx);
    EVP_CIPHER_CTX_init(&ctx);
    EVP_EncryptInit(&ctx, EVP_aes_128_cbc(), key, mac);

    for (i = 0; 1; i+=16) {
        if (finish && data_len <= 16) break;

        EVP_EncryptUpdate(&ctx, mac, &cypherLen, data + i, 16);

        data_len -= 16;
        if (data_len == 0) break;
    }

    if (finish) {
        cmac_subkey(result, data_len == 16 ? 1 : 2);
        #if DEBUG
            printf("KX       ");
            print_hex(result, 16);
            printf("\n");
        #endif

        uint8_t *last_block = data + i;

        for (i = 0; i < data_len; i++) {
            result[i] ^= last_block[i];
        }

        if (i < 16) {
            result[i] ^= 128;
            for (i++; i < 16; i++) {
                result[i] ^= 0;
            }
        }

        EVP_EncryptUpdate(&ctx, mac, &cypherLen, result, 16);
    }

    #if DEBUG
        printf("AES_CMAC ");
        print_hex(mac, 16);
        printf("\n");
    #endif

    EVP_CIPHER_CTX_cleanup(&ctx);
}

/* Private Funktionen ------------------------------------------------------ */

void getBlockKey(uint8_t *out, uint8_t *key, uint8_t *nonce, uint32_t index) {
    uint8_t a[16];
    memset(a, 0, 16);

    a[0] = (LEN_LEN - 1);
    memcpy(a + 1, nonce, NONCE_LEN);
    uint32_t i;
    for (i = 15; i > NONCE_LEN; i--) {
        a[i] = (index >> ((15-i)*8)) & 0xFF;
    }

    #if DEBUG
        printf("a[%u] Block für CCM:", index);
        for (i = 0; i < 16; i++) printf(" %02X", a[i]);
        printf("\n");
    #endif

    EVP_CIPHER_CTX ctx;
    EVP_EncryptInit(&ctx, EVP_aes_128_ecb(), key, 0);
    EVP_CIPHER_CTX_set_padding(&ctx, 0);
    int32_t length;
    EVP_EncryptUpdate(&ctx, out, &length, a, 16);
    EVP_CIPHER_CTX_cleanup(&ctx);
}

void setAuthCode(uint8_t data[], size_t data_len, uint8_t key[16], uint8_t nonce[NONCE_LEN]) {
    size_t i;

    // b_0 generieren
    uint8_t b_0[16];
    memset(b_0, 0, 16);
    // Flags
    b_0[0] = (8 * ((MAC_LEN-2)/2)) + (LEN_LEN - 1);
    // Nonce
    memcpy(b_0 + 1, nonce, NONCE_LEN);
    // Länge der Nachricht
    for (i = 15; i > NONCE_LEN; i--) {
        b_0[i] = (data_len >> ((15-i)*8)) & 0xFF;
    }

    #if DEBUG
        printf("b_0 Block für CCM:");
        for (i = 0; i < 16; i++) printf(" %02X", b_0[i]);
        printf("\n");
    #endif

    uint8_t cypher[16];
    int32_t cypherLen;

    EVP_CIPHER_CTX ctx;
    EVP_CIPHER_CTX_init(&ctx);
    EVP_EncryptInit(&ctx, EVP_aes_128_cbc(), key, NULL);
    EVP_CIPHER_CTX_set_padding(&ctx, 0);

    EVP_EncryptUpdate(&ctx, cypher, &cypherLen, b_0, 16);

    uint8_t plaintext[16];
    for (i = 0; i < data_len; i+=16) {
        memset(plaintext, 0, 16);
        memcpy(plaintext, data + i, min(16, data_len - i));
        EVP_EncryptUpdate(&ctx, cypher, &cypherLen, plaintext, 16);
    }

    memcpy(data + data_len, cypher, MAC_LEN);
    EVP_CIPHER_CTX_cleanup(&ctx);

    uint8_t s[16];
    // A-Block verschlüssel und mit dem bereits berechneten MAC X-Oren
    getBlockKey(s, key, nonce, 0);
    for (i = 0; i < MAC_LEN; i++) data[data_len + i] ^= s[i];
}

void crypt(uint8_t data[], size_t data_len, uint8_t key[16], uint8_t nonce[NONCE_LEN]) {
    uint32_t i;
    uint8_t s[16];

    for (i = 0; i < data_len; i+=16) {
        getBlockKey(s, key, nonce, (i/16)+1);
        size_t j;
        size_t blocklen = min(16, data_len - i);
        for (j = 0; j < blocklen; j++) data[i+j] ^= s[j];
    }
}

__attribute__((always_inline)) static void cmac_subkey(uint8_t L[16], uint8_t K) {
    while (K > 0) {
        uint8_t i, msb = L[0] & 0x80;
        for (i = 0; i < 15; i++) {
            L[i] <<= 1;
            L[i] |= (L[i+1] >> 7);
        }
        L[15] <<= 1;
        if (msb) {
            for (i = 0; i < 15; i++) {
                L[i] ^= 0;
            }
            L[15] ^= 0x87;
        }
        K--;
    }
}
