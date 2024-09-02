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
/*  PORT SPECIFIC C INFORMATION                            RELEASE        */
/*                                                                        */
/*    nx_secure_port.h                                  Cortex-M4/IAR     */
/*                                                           5.2          */
/*                                                                        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file contains data type definitions for the specific platform. */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-09-2017     Timothy Stapko           Initial Version 5.10          */
/*  08-01-2017     Timothy Stapko           Modified comments, fixed a    */
/*                                            typo, resulting in          */
/*                                            version 5.10 SP1            */
/*  09-20-2017     Timothy Stapko           Modified comments,            */
/*                                            resulting in version 5.1    */
/*  12-15-2017     Timothy Stapko           Modified comments,            */
/*                                            resulting in version 5.2    */
/*                                                                        */
/**************************************************************************/

#ifndef NX_SECURE_PORT_H
#define NX_SECURE_PORT_H


/* Determine if the optional NetX Seucre user define file should be used.  */


#define NX_SECURE_INCLUDE_USER_DEFINE_FILE

#ifdef NX_SECURE_INCLUDE_USER_DEFINE_FILE


/* Yes, include the user defines in nx_user.h. The defines in this file may
   alternately be defined on the command line.  */

#include "nx_secure_user.h"
#endif


#ifdef NX_SECURE_ENABLE_ECC_CIPHERSUITE
#include "nx_secure_tls_ecc.h"
#define NX_SECURE_ENABLE_AEAD_CIPHER
#define NX_SECURE_AEAD_CIPHER_CHECK(a) ((a) == NX_CRYPTO_ENCRYPTION_AES_GCM_16)
#endif /* NX_SECURE_ENABLE_ECC_CIPHERSUITE */

/* Define the version ID of NetX Secure.  This may be utilized by the application.  */

#ifdef NX_SECURE_SYSTEM_INIT
CHAR                            _nx_secure_version_id[] =
                                    "Copyright (c) 1996-2018 Express Logic Inc. * NetX Secure Synergy Version 5.11.1.5.2 SN: 4154-280-5000 *";
#else
extern  CHAR                    _nx_secure_version_id[];
#endif

#endif

