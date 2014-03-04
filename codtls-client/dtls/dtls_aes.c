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

void aes_cmac_init(CMAC_CTX *ctx, uint8_t *key, size_t key_length) {
    ctx->buf_pos = 0;
    memset(ctx->mac, 0, 16);

    if (key_length == 16) {
        memcpy(ctx->key, key, 16);
        #if DEBUG
            printf("Key16    ");
            print_hex(ctx->key, 16);
            printf("\n");
        #endif
        return;
    }

    memset(ctx->key, 0, 16);
    aes_cmac_update(ctx, key, key_length);
    aes_cmac_finish(ctx, ctx->key, 16);

    ctx->buf_pos = 0;
    memset(ctx->mac, 0, 16);

    #if DEBUG
        printf("KeyXX    ");
        print_hex(ctx->key, 16);
        printf("\n");
    #endif
}

void aes_cmac_update(CMAC_CTX *ctx, uint8_t *data, size_t data_len) {
    uint32_t i = 0;
    int32_t cypherLen;

    EVP_CIPHER_CTX actx;
    EVP_CIPHER_CTX_init(&actx);
    EVP_EncryptInit(&actx, EVP_aes_128_cbc(), ctx->key, ctx->mac);

    while (data_len > 0 && ctx->buf_pos < 16) {
      ctx->buf[ctx->buf_pos++] = data[i++];
      data_len -= 1;
    }
    if (data_len == 0) return;

    if (data_len > 0) {
      EVP_EncryptUpdate(&actx, ctx->mac, &cypherLen, ctx->buf, 16);
    }

    for (; data_len > 16; i+=16) {
        EVP_EncryptUpdate(&actx, ctx->mac, &cypherLen, data + i, 16);
        data_len -= 16;
    }
    memcpy(ctx->buf, data + i, data_len);
    ctx->buf_pos = data_len;
}

void aes_cmac_finish(CMAC_CTX *ctx, uint8_t *mac, size_t mac_len) {
    uint32_t i;
    int32_t cypherLen;

    EVP_CIPHER_CTX actx;
    EVP_CIPHER_CTX_init(&actx);
    EVP_EncryptInit(&actx, EVP_aes_128_cbc(), ctx->key, NULL);

    // Calculate Subkey - BEGIN
    uint8_t subkey[16];
    memset(subkey, 0, 16);
    EVP_EncryptUpdate(&actx, subkey, &cypherLen, subkey, 16);
    #if DEBUG
        printf("K0       ");
        print_hex(subkey, 16);
        printf("\n");
    #endif
    cmac_subkey(subkey, ctx->buf_pos == 16 ? 1 : 2);
    #if DEBUG
        printf("KX       ");
        print_hex(subkey, 16);
        printf("\n");
    #endif
    // Calculate Subkey - END

    for (i = 0; i < ctx->buf_pos; i++) {
        subkey[i] ^= ctx->buf[i];
    }

    if (i < 16) subkey[i] ^= 128;

    EVP_CIPHER_CTX_init(&actx);
    EVP_EncryptInit(&actx, EVP_aes_128_cbc(), ctx->key, ctx->mac);
    EVP_EncryptUpdate(&actx, ctx->mac, &cypherLen, subkey, 16);
    memcpy(mac, ctx->mac, mac_len);

    ctx->buf_pos = 0;
    memset(ctx->mac, 0, 16);

    #if DEBUG
        printf("AES_CMAC ");
        print_hex(mac, mac_len);
        printf("\n");
    #endif
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
