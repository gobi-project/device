/* __DTLS_H__ */
#ifndef __DTLS_H__
#define __DTLS_H__

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
//#include <netinet/in.h>

/* Record Layer Datenstrukturen -------------------------------------------- */

typedef enum {
    type_8_bit = 0,
    alert = 1,
    handshake = 2,
    application_data = 3
} RecordType;

typedef enum {
    dtls_1_0 = 0,
    version_16_bit = 1,
    dtls_1_2 = 2,
    version_future_use = 3
} Version;

typedef enum {
    epoch_0 = 0,
    epoch_1 = 1,
    epoch_2 = 2,
    epoch_3 = 3,
    epoch_4 = 4,
    epoch_8_bit = 5,
    epoch_16_bit = 6,
    epoch_implicit = 7 // same as previous record in the datagram
} Epoch;

typedef enum {
    snr_0 = 0,
    snr_8_bit = 1,
    snr_16_bit = 2,
    snr_24_bit = 3,
    snr_32_bit = 4,
    snr_40_bit = 5,
    snr_48_bit = 6,
    snr_implicit = 7 // number of previous record in the datagram + 1
} SequenceNumber;

typedef enum {
    rec_length_0 = 0,
    rec_length_8_bit = 1,
    rec_length_16_bit = 2,
    rec_length_implicit = 3 // datagram size - sizeof(DTLSRecord_t) or last datagram in record
} RecordLength;

typedef struct {
    Epoch epoch:3;
    Version version:2;
    RecordType type:2;
    uint8_t u1:1; // unbenutzt
    RecordLength length:2;
    SequenceNumber snr:3;
    uint8_t u2:3; // unbenutzt
    uint8_t payload[0];
} __attribute__ ((packed)) DTLSRecord_t;

/* ------------------------------------------------------------------------- */

/**
  * \brief  Funktion für den Datenversand über DTLS
  *
  *         Funktion für den Datenversand über DTLS mit dem gleichen Verhalten
  *         wie die bekannte sendto()-Funktion. Die notwendigen Session-Daten
  *         werden dabei automatisch anhand der IP-Adresse ermittelt.
  */
ssize_t dtls_sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen);

/**
  * \brief  Funktion für den Datenempfang über DTLS
  *
  *         Funktion für den Datenempfang über DTLS mit dem gleichen Verhalten
  *         wie die bekannte recvfrom()-Funktion. Die notwendigen Session-Daten
  *         werden dabei automatisch anhand der IP-Adresse ermittelt.
  */
ssize_t dtls_recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen);

#endif /* __DTLS_H__ */
