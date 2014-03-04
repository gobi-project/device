/* __DTLS_PRF_H__ */
#ifndef __DTLS_PRF_H__
#define __DTLS_PRF_H__

#include <stddef.h>
#include <stdint.h>

/**
  * \brief  CMAC based pseudo random function
  *
  *         Pseudo random function like used in (D)TLS with
  *         the difference, that CMAC is used instead of HMAC.
  *
  *         PRF(secret, label, seed) = P_CMAC(secret, label + seed)
  *
  *         P_CMAC(secret, seed) = CMAC(secret, A(1) + seed) +
  *                                CMAC(secret, A(2) + seed) +
  *                                CMAC(secret, A(3) + seed) + ...
  *         A(0) = seed
  *         A(i) = CMAC(secret, A(i-1))
  *
  * \param  dst         destination where random output is placed
  * \param  len         length of the needed random output
  * \param  data        includes the secret concatenated with seed
  * \param  seed_len    length of the secret
  * \param  seed_len    length of the seed
  */
void prf(uint8_t *dst, uint8_t len, uint8_t *data, size_t secret_len, size_t seed_len);

#endif /* __DTLS_PRF_H__ */


