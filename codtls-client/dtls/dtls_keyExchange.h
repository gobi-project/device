/* __DTLS_KEYEXCHANGE_H__ */
#ifndef __DTLS_KEYEXCHANGE_H__
#define __DTLS_KEYEXCHANGE_H__

#include <stdint.h>

typedef enum {
    explicit_prime = 1,
    explicit_char2 = 2,
    named_curve = 3
    // reserved(248..255)
    // max = 255
} __attribute__ ((packed)) ECCurveType;

// Schon in Network Byte Order hinterlegt
typedef enum {
    sect163k1 = 0x0100,
    sect163r1 = 0x0200,
    sect163r2 = 0x0300,
    sect193r1 = 0x0400,
    sect193r2 = 0x0500,
    sect233k1 = 0x0600,
    sect233r1 = 0x0700,
    sect239k1 = 0x0800,
    sect283k1 = 0x0900,
    sect283r1 = 0x1000,
    sect409k1 = 0x1100,
    sect409r1 = 0x1200,
    sect571k1 = 0x1300,
    sect571r1 = 0x1400,
    secp160k1 = 0x1500,
    secp160r1 = 0x1600,
    secp160r2 = 0x1700,
    secp192k1 = 0x1800,
    secp192r1 = 0x1900,
    secp224k1 = 0x2000,
    secp224r1 = 0x2100,
    secp256k1 = 0x2200,
    secp256r1 = 0x2300,
    secp384r1 = 0x2400,
    secp521r1 = 0x2500,
    // reserved = 0x00fe..0xfffe     0xAABB AA zählt hoch wegen NBO
    arbitrary_explicit_prime_curves = 0x01ff,
    arbitrary_explicit_char2_curves = 0x02ff,
    // max = 0xffff
} __attribute__ ((packed)) NamedCurve;

typedef struct {
    ECCurveType curve_type;
    NamedCurve namedcurve;
} __attribute__ ((packed)) ECParameters;

typedef enum {
    compressed = 2,
    uncompressed = 4,
    hybrid = 6
} __attribute__ ((packed)) PointType;

typedef struct {
    uint8_t len;     // 0x41 = 65 Lang
    PointType type;  // 0x04 uncompressed
    uint32_t x[8];
    uint32_t y[8];
} __attribute__ ((packed)) ECPoint;

typedef struct { // 2 + 16 + 3 + 66 = 87 Byte groß
    uint16_t pskHint_len;   // 16
    uint8_t pskHint[16];
    ECParameters curve_params;
    ECPoint public_key;
} __attribute__ ((packed)) KeyExchange_t;

#endif /* __DTLS_KEYEXCHANGE_H__ */
