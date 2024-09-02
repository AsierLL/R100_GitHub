/******************************************************************************
 * Name      : S3A7_REANIBEX_100                                              *
 * Copyright : Osatu S. Coop                                                  *
 * Compiler  : MinGW32                                                        *
 * Target    : Reanibex Series                                                *
 ******************************************************************************/

/*!
 * @file        secure_crypto.h
 * @brief       Header with functions related to the system monitor services
 *
 * @version     v1
 * @date        08/10/2021
 * @author      ivicente
 * @warning     author   date    version     .- Edition
 * @bug
 *
 */

/******************************************************************************
 ** Includes
 */

#include "nx_secure_x509.h"

#include "HAL/thread_comm_hal.h"
#include "device_init.h"
#include "resources/Trace.h"

#include "R100_Errors.h"
#include "types_basic.h"
#include "types_app.h"
#include "Trace.h"

#include "HAL/thread_defibrillator_hal.h"
#include "HAL/thread_patMon_hal.h"
#include "HAL/thread_audio_hal.h"
#include "thread_comm_entry.h"
#include "thread_patMon_entry.h"
#include "thread_sysMon_entry.h"
#include "thread_sysMon.h"
#include "thread_audio_entry.h"
#include "thread_audio.h"
#include "thread_core_entry.h"
#include "thread_core.h"

#include "thread_comm.h"
#include "thread_api.h"

#include "sha256.h"
#include <Bexlib.h>

/******************************************************************************
 ** Defines
 */

#define     DER_Size_Limit      2048

#define     CERT_VALID          0       ///< The ertificate is valid
#define     CERT_EXP_SOON       1       ///< The certificate will expire soon
#define     CERT_NOT_VALID      2       ///< The ertificate is not valid

#define     CERT_FOLDER_NAME            "CERT"
#define     WPA_EAP_CA_CERT_FILENAME    "client_router_ca.der"  ///< Es el ca.der copiado de Docker
#define     WPA_EAP_CERT_FILENAME       "client_router.crt"     ///< Es el client.der copiado de Docker
#define     WPA_EAP_CERT_KEY_FILENAME   "client_router_key.key" ///< Es la llave del cliente copiado de Docker
#define     TLS_CA_CERT_FILENAME        "server_tls_ca.der"     ///< ca.der creado por mi del servidor

#define     DIGEST                      "digest.txt"
#define     SIGNATURE                   "signature.txt"
#define     PUBLIC_KEY                  "public_key.txt"
#define     USB_KEY                     "things01.dat"

#define     CRYPTO_SIZE                 256
#define     WIFI_SIZE                   330

/******************************************************************************
 ** Prototypes
 */

void            NX_Init                     (void);
uint16_t        Check_TLS_Cacert            (void);
uint16_t        Check_WPA_EAP_Cert          (void);
void            Check_Certs                 (bool_t* eap, bool_t* tls);

bool_t          Crypto_ATECC_IsPresent      (void);
bool_t          Is_ATECC_Initialized        (void);

ATCA_STATUS     Crypto_ATECC_Init           (void);
ATCA_STATUS     Crypto_ATECC_Config         (void);
ATCA_STATUS     Crypto_ATECC_Encrypt_File   (void);
ATCA_STATUS     Crypto_ATECC_Encrypt_NFC    (uint8_t *r_w_data);
ATCA_STATUS     Crypto_ATECC_Decrypt        (uint8_t *r_w_data);
ATCA_STATUS     Crypto_ATECC_Decrypt_File   (void);
ATCA_STATUS     Crypto_ATECC_Inject         (void);
ATCA_STATUS     Crypto_ATECC_Inject_Zero    (void);
ATCA_STATUS     Crypto_ATECC_Verify         (bool *is_verified);
ATCA_STATUS     Crypto_ATECC_SHA256         (uint8_t *digest);
ATCA_STATUS     Crypto_ATECC_SHA256_HMAC    (uint8_t *digest);

ATCA_STATUS     Crypto_ATECC_Verify_Test    (bool *is_verified);
/*
void            Encrypt_Wifi_File       (void);
void            Decrypt_Wifi_File       (void);
*/

void            SHA2_SHA256             (char_t* data_hex);
void            SHA2_SHA256_HMAC        (char_t* data_hex);

void            AES_Algo_Encrypt_File   (void);
void            AES_Algo_Decrypt_File   (void);

void            AES_Algo_Decrypt        (char_t* r_w_data);

bool_t          AES_Algo_Verify         (void);