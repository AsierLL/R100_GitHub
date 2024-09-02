/* generated configuration header file - do not edit */
#ifndef NX_SECURE_USER_H_
#define NX_SECURE_USER_H_
/* NX_SECURE_TLS_DISABLE_TLS_1_1 defines whether or not to disable TLS 1.1 protocol support.
 BY default TLS 1.1 is supported. */
#if (1)
#define NX_SECURE_TLS_DISABLE_TLS_1_1
#endif

/* NX_SECURE_TLS_ENABLE_TLS_1_0 defines whether or not to enable TLS 1.0 protocol support.
 BY default TLS 1.0 is not supported. */
#if (0)
#define NX_SECURE_TLS_ENABLE_TLS_1_0
#endif

/* Defines Maximum RSA Modulus Bits*/
#ifndef NX_CRYPTO_MAX_RSA_MODULUS_SIZE
#define NX_CRYPTO_MAX_RSA_MODULUS_SIZE                (4096U)
#endif

/* Enables the optional X.509 Distinguished
 Name fields, at the expense of extra memory use for X.509 certificates.*/
#if (0)
#define NX_SECURE_X509_USE_EXTENDED_DISTINGUISHED_NAMES
#endif

/* Enables strict distinguished name comparison for X.509 certificates for certificate searching and verification*/
#if (0)
#define NX_SECURE_X509_STRICT_NAME_COMPARE
#endif

/* NX_SECURE_ENABLE_PSK_CIPHERSUITES enables Pre-Shared Key.  By default
 this feature is not enabled. */
#if (0)
#define NX_SECURE_ENABLE_PSK_CIPHERSUITES
#endif

/* NX_SECURE_ENABLE_ECC_CIPHERSUITES enables ECC ciphersuites.  By default
 this feature is not enabled. */
#if (0)
#define NX_SECURE_ENABLE_ECC_CIPHERSUITE
#else
#define NX_SECURE_DISABLE_ECC_CIPHERSUITE
#endif

/* This option removes all TLS stack code related to TLS Server mode, reducing code and data usage */
#if (0)
#define NX_SECURE_TLS_SERVER_DISABLED
#endif

/* This option removes all TLS stack code related to TLS Client mode, reducing code and data usage */
#if (0)
#define NX_SECURE_TLS_CLIENT_DISABLED
#endif

/* This option allows TLS to accept self-signed certificates from a remote host. 
 Otherwise the certificate must be issued by a certificate in the trusted certificate store */
#if (0)
#define NX_SECURE_ALLOW_SELF_SIGNED_CERTIFICATES
#endif        

/* This option specifies whether to use Software Crypto Engines or not */
#if (0)
#define NX_CRYPTO_ENGINE_SW
#endif
#endif /* NX_SECURE_USER_H_ */
