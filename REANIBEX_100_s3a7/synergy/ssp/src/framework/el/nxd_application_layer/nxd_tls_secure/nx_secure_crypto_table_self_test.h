/**************************************************************************/
/*                                                                        */
/*            Copyright (c) 1996-2018 by Express Logic Inc.               */
/*                                                                        */
/*  This software is copyrighted by and is the sole property of Express   */
/*  Logic, Inc.  All rights, title, ownership, or other interests         */
/*  in the software remain the property of Express Logic, Inc.  This      */
/*  software may only be used in accordance with the corresponding        */
/*  license agreement.  Any unauthorized use, duplication, transmission,  */
/*  distribution, or disclosure of this software is expressly forbidden.  */
/*                                                                        */
/*  This Copyright notice may not be removed or modified without prior    */
/*  written consent of Express Logic, Inc.                                */
/*                                                                        */
/*  Express Logic, Inc. reserves the right to modify this software        */
/*  without notice.                                                       */
/*                                                                        */
/*  Express Logic, Inc.                     info@expresslogic.com         */
/*  11423 West Bernardo Court               http://www.expresslogic.com   */
/*  San Diego, CA  92127                                                  */
/*                                                                        */
/**************************************************************************/


/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/** NetX Secure Component                                                 */
/**                                                                       */
/**    Transport Layer Security (TLS)                                     */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  COMPONENT DEFINITION                                   RELEASE        */
/*                                                                        */
/*    nx_secure_crypto_table_self_test.h                  PORTABLE C      */
/*                                                           5.11 SP1     */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines all services for module self-test.                */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  07-15-2018     Timothy Stapko           Initial Version 5.10SP1       */
/*                                                                        */
/**************************************************************************/

#ifndef NX_SECURE_CRYPTO_TABLE_SELF_TEST_H
#define NX_SECURE_CRYPTO_TABLE_SELF_TEST_H

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */
#ifdef __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {

#endif

#include "nx_secure_tls.h"

UINT _nx_secure_crypto_table_self_test(const NX_SECURE_TLS_CRYPTO *crypto_table,
                                       VOID *metadata, UINT metadata_size);

#ifdef NX_SECURE_POWER_ON_SELF_TEST_MODULE_INTEGRITY_CHECK
/* This structure is used for self test function lookup table. */
typedef struct NX_SECURE_CRYPTO_SELF_TEST_STRUCT
{

    /* Algorithm ID. */
    UINT nx_crypto_algorithm;

    /* Function pointer to the self test function for the corresponding algorithm. */
    UINT (*self_test_function)(NX_CRYPTO_METHOD *crypto_method, VOID *metadata, UINT metadata_size);

} NX_SECURE_CRYPTO_SELF_TEST;

UINT _nx_secure_crypto_method_self_test_aes(NX_CRYPTO_METHOD *crypto_method_aes,
                                            VOID *metadata, UINT metadata_size);
UINT _nx_secure_crypto_method_self_test_des(NX_CRYPTO_METHOD *crypto_method_des,
                                            VOID *metadata, UINT metadata_size);
UINT _nx_secure_crypto_method_self_test_3des(NX_CRYPTO_METHOD *crypto_method_3des,
                                             VOID *metadata, UINT metadata_size);
UINT _nx_secure_crypto_method_self_test_sha(NX_CRYPTO_METHOD *crypto_method_sha,
                                            VOID *metadata, UINT metadata_size);
UINT _nx_secure_crypto_method_self_test_md5(NX_CRYPTO_METHOD *crypto_method_md5,
                                            VOID *metadata, UINT metadata_size);
UINT _nx_secure_crypto_method_self_test_rsa(NX_CRYPTO_METHOD *crypto_method,
                                            VOID *metadata, UINT metadata_size);
UINT _nx_secure_crypto_method_self_test_hmac_sha(NX_CRYPTO_METHOD *crypto_method_hmac_sha,
                                                 VOID *metadata, UINT metadata_size);
UINT _nx_secure_crypto_method_self_test_hmac_md5(NX_CRYPTO_METHOD *crypto_method_hmac_md5,
                                                 VOID *metadata, UINT metadata_size);
UINT _nx_secure_crypto_method_self_test_prf(NX_CRYPTO_METHOD *crypto_method_prf,
                                            VOID *metadata, UINT metadata_size);
#endif
#endif /* NX_SECURE_CRYPTO_TABLE_SELF_TEST_H */

