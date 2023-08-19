#ifndef MYC_PBKDF_H
#define MYC_PBKDF_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * PBKDF2_SHA256(passwd, passwdlen, salt, saltlen, c, buf, dkLen):
 * Compute PBKDF2(passwd, salt, c, dkLen) using HMAC-SHA256 as the PRF, and
 * write the output to buf.  The value dkLen must be at most 32 * (2^32 - 1).
 */
void myc_pbkdf2_sha256(const uint8_t * passwd, size_t passwdlen, const uint8_t * salt, size_t saltlen, uint64_t c, uint8_t * buf, size_t dkLen);

/**
 * PBKDF2_SHA3-256(passwd, passwdlen, salt, saltlen, c, buf, dkLen):
 * Compute PBKDF2(passwd, salt, c, dkLen) using HMAC-SHA256 as the PRF, and
 * write the output to buf.  The value dkLen must be at most 32 * (2^32 - 1).
 */
void myc_pbkdf2_sha3_256(const uint8_t * passwd, size_t passwdlen, const uint8_t * salt, size_t saltlen, uint64_t c, uint8_t * buf, size_t dkLen);

/**
 * PBKDF2_SHA512(passwd, passwdlen, salt, saltlen, c, buf, dkLen):
 * Compute PBKDF2(passwd, salt, c, dkLen) using HMAC-SHA512 as the PRF, and
 * write the output to buf.  The value dkLen must be at most 32 * (2^32 - 1).
 */
void myc_pbkdf2_sha512(const uint8_t * passwd, size_t passwdlen, const uint8_t * salt, size_t saltlen, uint64_t c, uint8_t * buf, size_t dkLen);

/**
 * PBKDF2_SHA3_512(passwd, passwdlen, salt, saltlen, c, buf, dkLen):
 * Compute PBKDF2(passwd, salt, c, dkLen) using HMAC-SHA3-512 as the PRF, and
 * write the output to buf.  The value dkLen must be at most 32 * (2^32 - 1).
 */
void myc_pbkdf2_sha3_512(const uint8_t * passwd, size_t passwdlen, const uint8_t * salt, size_t saltlen, uint64_t c, uint8_t * buf, size_t dkLen);

#ifdef __cplusplus
}
#endif


#endif
