/* __DTLS_CCM_H__ */
#ifndef __DTLS_CCM_H__
#define __DTLS_CCM_H__

#include <stdlib.h>
#include <stdint.h>

#define MAC_LEN 8                 // Länge des Authentication Fields    Element von {4, 6, 8, 10, 12, 14, 16}
#define LEN_LEN 3                 // Länge des Längenfeldes             Element von {2, 3, 4, 5, 6, 7, 8}
#define NONCE_LEN (15-LEN_LEN)    // Es Ergibt sich die Länge der Nonce

/**
  * \brief  Verschlüsselung
  *
  *         Verschlüsselt die in data hinterlegten Daten mit key und nonce.
  *         Die Daten werden verschlüsselt und ein 8 Byte langer MAC wird
  *         angehangen. Genug Speicher muss reserviert sein.
  *
  * \param  data        Zeiger auf die Struktur mit Nonce und Klartextdaten
  * \param  data_len    Länge der Klartextdaten
  * \param  key         Schlüssel mit dem die Daten verschlüsselt und der MAC erzeugt wird
  * \param  nonce       Nonce die zur Verschlüsselung der Daten herangezogen wird
  */
void aes_encrypt(uint8_t data[], size_t data_len, uint8_t key[16], uint8_t nonce[NONCE_LEN]);

/**
  * \brief    Entschlüsselung
  *
  *           Entschlüsselt die in datahinterlegten Daten mit key und nonce.
  *           Die Daten werden entschlüsselt und ein 8 Byte langer MAC wird
  *           angehangen, wobei der der alte MAC überschrieben wird.
  *
  * \param  data        Zeiger auf die Struktur mit Nonce und Klartextdaten
  * \param  data_len    Länge der Klartextdaten
  * \param  key         Schlüssel mit dem die Daten entschlüsselt und der MAC erzeugt wird
  * \param  nonce       Nonce die zur Entschlüsselung der Daten herangezogen wird
  */
void aes_decrypt(uint8_t data[], size_t data_len, uint8_t key[16], uint8_t nonce[NONCE_LEN]);

typedef struct {
    uint8_t key[16];
    uint8_t mac[16];
    uint8_t buf[16];
    size_t buf_pos;
} CMAC_CTX;

/**
  * \brief  CMAC initialisation
  *
  *         CMAC implementation for
  *         http://tools.ietf.org/html/rfc4493
  *         http://tools.ietf.org/html/rfc4494
  *         http://tools.ietf.org/html/rfc4615
  *
  *         Befor calculating a cmac its important to reserve memory for
  *         CMAC_CTX and call this function die initialize the context
  *         and include the key.
  *
  * \param  ctx        Pointer to CMAC_CTX needed for calculation
  * \param  key        Pointer to the key
  * \param  key_len    Length of the key
  */
void aes_cmac_init(CMAC_CTX *ctx, uint8_t *key, size_t key_length);

/**
  * \brief  CMAC initialisation
  *
  *         CMAC implementation for
  *         http://tools.ietf.org/html/rfc4493
  *         http://tools.ietf.org/html/rfc4494
  *         http://tools.ietf.org/html/rfc4615
  *
  *         After initialisation u can call this function as often as needed
  *         to include more data into cmac calculation.
  *
  * \param  ctx        Pointer to CMAC_CTX needed for calculation
  * \param  data       Pointer to the data
  * \param  data_len   Length of the data
  */
void aes_cmac_update(CMAC_CTX *ctx, uint8_t *data, size_t data_len);

/**
  * \brief  CMAC initialisation
  *
  *         CMAC implementation for
  *         http://tools.ietf.org/html/rfc4493
  *         http://tools.ietf.org/html/rfc4494
  *         http://tools.ietf.org/html/rfc4615
  *
  *         After update its important to call this function. It will
  *         output the final cmac to mac.
  *
  * \param  ctx        Pointer to CMAC_CTX needed for calculation
  * \param  data       Pointer to the memory for cmac
  * \param  data_len   Length of the needed mac
  */
void aes_cmac_finish(CMAC_CTX *ctx, uint8_t *mac, size_t mac_len);

#endif /* __DTLS_CCM__ */
