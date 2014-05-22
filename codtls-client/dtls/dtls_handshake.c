#include "dtls_handshake.h"

#include <string.h>
#include <time.h>

#include "dtls_ecc.h"
#include "dtls_random.h"
#include "dtls_content.h"
#include "dtls_clientHello.h"
#include "dtls_serverHello.h"
#include "dtls_keyExchange.h"
#include "dtls_data.h"
#include "dtls_prf.h"
#include "dtls_aes.h"
#include "../coap_client.h"

#define DEBUG 0
#define DEBUG_ECC 0
#define DEBUG_PRF 0
#define DEBUG_FIN 0

#if DEBUG || DEBUG_ECC || DEBUG_PRF || DEBUG_FIN
    #include <stdio.h>
#endif

#if DEBUG
    #define PRINTF(...) printf(__VA_ARGS__)
#else
    #define PRINTF(...)
#endif

uint32_t base_x[8] = {0xd898c296, 0xf4a13945, 0x2deb33a0, 0x77037d81, 0x63a440f2, 0xf8bce6e5, 0xe12c4247, 0x6b17d1f2};
uint32_t base_y[8] = {0x37bf51f5, 0xcbb64068, 0x6b315ece, 0x2bce3357, 0x7c0f9e16, 0x8ee7eb4a, 0xfe1a7f9b, 0x4fe342e2};

uint8_t handshake_messages[4096];
uint16_t handshake_messages_len;

extern uint8_t isHandshakeMessage;

void dtls_handshake(uint8_t ip[16]) {
    handshake_messages_len = 0;

    isHandshakeMessage = 1;

    uint32_t i;
    uint8_t len;
    uint8_t message[128];

    time_t my_time = time(NULL);
    uint8_t random[32];
    random[0] = (my_time >> 24) & 0xFF;
    random[1] = (my_time >> 16) & 0xFF;
    random[2] = (my_time >>  8) & 0xFF;
    random[3] = (my_time >>  0) & 0xFF;
    random_x(random + 4, 28);
    char buffer[256];

    len = makeClientHello(message, my_time, random + 4, NULL, 0);
    memset(buffer, 0, 256);
    PRINTF("Länge der Anfrage: %u Byte\n", len);
    coap_setPayload(message, len);
    coap_setBlock1(0, 1, 1);
    if (coap_request(ip, COAP_REQUEST_POST, "dtls", buffer) != 2) return;
    coap_setPayload(message, len);
    coap_setBlock1(1, 0, 1);
    if (coap_request(ip, COAP_REQUEST_POST, "dtls", buffer) != 4) return;
    if (getContentType(buffer) != hello_verify_request) {
        return;
    }
    HelloVerifyRequest_t *verify = (HelloVerifyRequest_t *) (getContentData(buffer));
    printf("Step 1 done: Cookie erhalten: ");
    for (i = 0; i < verify->cookie_len; i++) printf("%02X", verify->cookie[i]);
    printf("\n");

// --------------------------------------------------------------------------------------------

    len = makeClientHello(message, my_time, random + 4, verify->cookie, verify->cookie_len);

    memcpy(handshake_messages + handshake_messages_len, message, len);
    handshake_messages_len += len;

    memset(buffer, 0, 256);
    PRINTF("Länge der Anfrage: %u Byte\n", len);
    coap_setPayload(message, len);
    coap_setBlock1(0, 1, 1);
    if (coap_request(ip, COAP_REQUEST_POST, "dtls", buffer) != 2) return;
    coap_setPayload(message, len);
    coap_setBlock1(1, 1, 1);
    if (coap_request(ip, COAP_REQUEST_POST, "dtls", buffer) != 2) return;
    coap_setPayload(message, len);
    coap_setBlock1(2, 0, 1);
    if (coap_request(ip, COAP_REQUEST_POST, "dtls", buffer) != 2) return;
    if (getContentType(buffer) != server_hello) {
        PRINTF("Erwartetes ServerHello nicht erhalten. Abbruch.\n");
        return;
    }

    char *pointer = buffer;
    for (i = 0; i < 3; i++) {
        size_t len = getContentLen(pointer);
        memcpy(handshake_messages + handshake_messages_len, pointer, len);
        handshake_messages_len += len;
        pointer += len;
    }

    ServerHello_t *serverHello = (ServerHello_t *) (getContentData(buffer));
    printf("Step 2 done: Session-Id: %.*s\n", serverHello->session_id.len, serverHello->session_id.session_id);

    createSession(ip, serverHello->session_id.session_id);

// --------------------------------------------------------------------------------------------

    uint32_t result_x[8];
    uint32_t result_y[8];
    uint32_t private_key[8];
    do {
        random_x((uint8_t *) private_key, 32);
    } while (!ecc_is_valid_key(private_key));
    #if DEBUG_ECC
        printf("Private Key : ");
        for (i = 0; i < 8; i++) printf("%08X", htonl(private_key[i]));
        printf("\n");
    #endif

// --------------------------------------------------------------------------------------------

    KeyExchange_t *ske = (KeyExchange_t *) getContentData(getContent(buffer, 256, server_key_exchange));

    PRINTF("PSK-Hint erhalten: ");
    for (i = 0; i < ntohs(ske->pskHint_len); i++) PRINTF("%02X", ske->pskHint[i]);
    PRINTF("\n");

    uint8_t psk[16];
    getPSK(psk, ske->pskHint);
    PRINTF("PSK ermittelt: ");
    for (i = 0; i < 16; i++) PRINTF("%02X", psk[i]);
    PRINTF("\n");

    #if DEBUG_ECC
        printf("_S_PUB_KEY-X: ");
        for (i = 0; i < 8; i++) printf("%08X", ntohl(ske->public_key.x[i]));
        printf("\n_S_PUB_KEY-Y: ");
        for (i = 0; i < 8; i++) printf("%08X", ntohl(ske->public_key.y[i]));
        printf("\n");
    #endif
    uint8_t *bufx;
    uint8_t *bufy;
    bufx = (uint8_t *) ske->public_key.x;
    bufy = (uint8_t *) ske->public_key.y;
    for (i = 0; i < 16; i++) {
        bufx[     i] ^= bufx[31 - i];
        bufx[31 - i] ^= bufx[     i];
        bufx[     i] ^= bufx[31 - i];

        bufy[     i] ^= bufy[31 - i];
        bufy[31 - i] ^= bufy[     i];
        bufy[     i] ^= bufy[31 - i];
    }
    ecc_ec_mult(ske->public_key.x, ske->public_key.y, private_key, result_x, result_y);
    bufx = (uint8_t *) result_x;
    bufy = (uint8_t *) result_y;
    for (i = 0; i < 16; i++) {
        bufx[     i] ^= bufx[31 - i];
        bufx[31 - i] ^= bufx[     i];
        bufx[     i] ^= bufx[31 - i];

        bufy[     i] ^= bufy[31 - i];
        bufy[31 - i] ^= bufy[     i];
        bufy[     i] ^= bufy[31 - i];
    }
    #if DEBUG_ECC
        printf("SECRET_KEY-X: ");
        for (i = 0; i < 8; i++) printf("%08X", htonl(result_x[i]));
        printf("\nSECRET_KEY-Y: ");
        for (i = 0; i < 8; i++) printf("%08X", htonl(result_y[i]));
        printf("\n");
    #endif

// --------------------------------------------------------------------------------------------

    ServerHello_t *sh = (ServerHello_t *) getContentData(getContent(buffer, 256, server_hello));

    uint8_t prf_buffer[170];
    prf_buffer[0] = 0;
    prf_buffer[1] = 16;
    memcpy(prf_buffer + 2, psk, 16);
    prf_buffer[18] = 0;
    prf_buffer[19] = 32;
    memcpy(prf_buffer + 20, result_x, 32);
    memcpy(prf_buffer + 52, "master secret", 13);
    memcpy(prf_buffer + 65, random, 32);           // Client-Random
    memcpy(prf_buffer + 97, &sh->random, 32);      // Server-Random
    #if DEBUG_PRF
        printf("Seed für Master-Secret:\n    ");
        for (i = 0; i < 20; i++) printf("%02X", prf_buffer[i]);
        printf("\n    ");
        for (i = 20; i < 52; i++) printf("%02X", prf_buffer[i]);
        printf("\n    ");
        for (i = 52; i < 65; i++) printf("%02X", prf_buffer[i]);
        printf("\n    ");
        for (i = 65; i < 97; i++) printf("%02X", prf_buffer[i]);
        printf("\n    ");
        for (i = 97; i < 129; i++) printf("%02X", prf_buffer[i]);
        printf("\n");
    #endif

    uint8_t master_secret[48];
    prf(master_secret, 48, prf_buffer, 52, 77);
    #if DEBUG_PRF
        printf("Master-Secret:\n    ");
        for (i = 0; i < 24; i++) printf("%02X", master_secret[i]);
        printf("\n    ");
        for (i = 24; i < 48; i++) printf("%02X", master_secret[i]);
        printf("\n");
    #endif

    memcpy(prf_buffer + 40, master_secret, 48);
    memcpy(prf_buffer + 88, "key expansion", 13);
    memcpy(prf_buffer + 101, &sh->random, 32);
    memcpy(prf_buffer + 133, random, 32);
    prf(prf_buffer, 40, prf_buffer + 40, 48, 77);
    #if DEBUG_PRF
        printf("Key-Block:\n    ");
        for (i = 0; i < 20; i++) printf("%02X", prf_buffer[i]);
        printf("\n    ");
        for (i = 20; i < 40; i++) printf("%02X", prf_buffer[i]);
        printf("\n");
    #endif

    insertKeyBlock(ip, (KeyBlock_t *) prf_buffer);

// --------------------------------------------------------------------------------------------

    KeyExchange_t cke;
    cke.pskHint_len = ske->pskHint_len;
    memcpy(cke.pskHint, ske->pskHint, ntohs(ske->pskHint_len));
    cke.curve_params.curve_type = named_curve;
    cke.curve_params.namedcurve = secp256r1;
    cke.public_key.len = 65;
    cke.public_key.type = uncompressed;
    #if DEBUG_ECC
        printf("BASE_POINT-X: ");
        for (i = 8; i > 0; i--) printf("%08X", base_x[i-1]);
        printf("\nBASE_POINT-Y: ");
        for (i = 8; i > 0; i--) printf("%08X", base_y[i-1]);
        printf("\n");
    #endif
        ecc_ec_mult(base_x, base_y, private_key, cke.public_key.x, cke.public_key.y);
        bufx = (uint8_t *) cke.public_key.x;
        bufy = (uint8_t *) cke.public_key.y;
        for (i = 0; i < 16; i++) {
            bufx[     i] ^= bufx[31 - i];
            bufx[31 - i] ^= bufx[     i];
            bufx[     i] ^= bufx[31 - i];

            bufy[     i] ^= bufy[31 - i];
            bufy[31 - i] ^= bufy[     i];
            bufy[     i] ^= bufy[31 - i];
        }
    #if DEBUG_ECC
        printf("_C_PUB_KEY-X: ");
        for (i = 0; i < 8; i++) printf("%08X", htonl(cke.public_key.x[i]));
        printf("\n_C_PUB_KEY-Y: ");
        for (i = 0; i < 8; i++) printf("%08X", htonl(cke.public_key.y[i]));
        printf("\n");
    #endif

    char uri[14];
    memcpy(uri, "dtls/", 5);
    memcpy(uri + 5, serverHello->session_id.session_id, serverHello->session_id.len);
    uri[13] = '\0';
    memset(buffer, 0, 256);
    uint8_t paylen = 0;
    paylen += makeContent(message, client_key_exchange, &cke, sizeof(KeyExchange_t));

    memcpy(handshake_messages + handshake_messages_len, message, paylen);
    handshake_messages_len += paylen;

    // Change Cipher Spec
    uint8_t changeCipherSpec = 1;
    paylen += makeContent(message + paylen, change_cipher_spec, &changeCipherSpec, 1);

    // Finished Nachrichten berechnen
    uint8_t finished_source[79];
    uint8_t client_finished[12];
    uint8_t server_finished[12];

    memset(finished_source + 63, 0, 16);
    CMAC_CTX ctx;
    aes_cmac_init(&ctx, psk, 16);
    aes_cmac_update(&ctx, handshake_messages, handshake_messages_len);
    aes_cmac_finish(&ctx, finished_source + 63, 16);
    memcpy(finished_source, master_secret, 48);

    memcpy(finished_source + 48, "client finished", 15);
    prf(client_finished, 12, finished_source, 48, 31);
    #if DEBUG_PRF
        printf("Client Finished: ");
        for (i = 0; i < 12; i++) printf("%02X", client_finished[i]);
        printf("\n");
    #endif

    size_t fin_len = makeContent(message + paylen, finished, client_finished, 12);

    // Client Finished für Server Finished zusätzlich berücksichtigen
    aes_cmac_update(&ctx, handshake_messages, handshake_messages_len);
    aes_cmac_update(&ctx, message + paylen, fin_len);
    aes_cmac_finish(&ctx, finished_source + 63, 16);
    // Client Finished für Server Finished zusätzlich berücksichtigen - ENDE

    uint8_t nonce[12];
    uint16_t epoch = getEpoch(ip) + 1;
    nonce[4] = (epoch & 0xFF00) >> 8;
    nonce[5] = (epoch & 0x00FF) >> 0;
    memset(nonce + 6, 0, 6);
    uint8_t *key_block = getKeyBlock(ip, epoch);
    memcpy(nonce, key_block + KEY_BLOCK_CLIENT_IV, 4);
    #if DEBUG_FIN
        printf("Nonce zum Verschlüsseln von Finished: ");
        for (i = 0; i < 12; i++) printf("%02X", nonce[i]);
        printf("\n");
        printf("Key zum Verschlüsseln von Finished: ");
        for (i = KEY_BLOCK_CLIENT_KEY; i < KEY_BLOCK_CLIENT_KEY + 16; i++) printf("%02X", key_block[i]);
        printf("\n");
    #endif
    aes_encrypt(message + paylen, fin_len, key_block + KEY_BLOCK_CLIENT_KEY, nonce);
    paylen += (fin_len + MAC_LEN);

    memcpy(finished_source + 48, "server finished", 15);
    prf(server_finished, 12, finished_source, 48, 31);
    #if DEBUG_PRF
        printf("Server Finished: ");
        for (i = 0; i < 12; i++) printf("%02X", server_finished[i]);
        printf("\n");
    #endif

    // Senden
    PRINTF("Länge der Anfrage: %u Byte\n", paylen);
    coap_setPayload(message, paylen);
    coap_setBlock1(0, 1, 1);
    if (coap_request(ip, COAP_REQUEST_POST, uri, buffer) != 2) return;
    coap_setPayload(message, paylen);
    coap_setBlock1(1, 1, 1);
    if (coap_request(ip, COAP_REQUEST_POST, uri, buffer) != 2) return;
    coap_setPayload(message, paylen);
    coap_setBlock1(2, 1, 1);
    if (coap_request(ip, COAP_REQUEST_POST, uri, buffer) != 2) return;
    coap_setPayload(message, paylen);
    coap_setBlock1(3, 0, 1);
    if (coap_request(ip, COAP_REQUEST_POST, uri, buffer) != 2) return;
    if (getContentType(buffer) != change_cipher_spec) {
        PRINTF("Erwartetes ChangeCipherSpec nicht erhalten. Abbruch.\n");
        return;
    }

    memcpy(nonce, key_block + KEY_BLOCK_SERVER_IV, 4);
    #if DEBUG_FIN
        printf("Nonce zum Entschlüsseln von Finished: ");
        for (i = 0; i < 12; i++) printf("%02X", nonce[i]);
        printf("\n");
        printf("Key zum Entschlüsseln von Finished: ");
        for (i = KEY_BLOCK_SERVER_KEY; i < KEY_BLOCK_SERVER_KEY + 16; i++) printf("%02X", key_block[i]);
        printf("\n");
    #endif
    aes_decrypt((uint8_t *) buffer + 3, 14, key_block + KEY_BLOCK_SERVER_KEY, nonce);

    #if DEBUG_FIN
        printf("Erhaltenes Server Finished: ");
        for (i = 5; i < 17; i++) printf("%02X", (uint8_t) buffer[i]);
        printf("\n");
    #endif

    printf("Step 3 done. Handshake abgeschlossen.\n");
    increaseEpoch(ip);

    isHandshakeMessage = 0;
}
