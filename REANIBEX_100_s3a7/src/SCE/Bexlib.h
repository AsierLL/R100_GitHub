/*
 * mylib.h
 *
 *  Created on: Jan 9, 2018
 *      Author: gjacobso01
 */
#include "cryptoauthlib.h"
#include "atca_status.h"
#include "atca_debug.h"
#include "atca_iface.h"
#include "atca_devtypes.h"
#include "atca_helpers.h"
#include "atca_cfgs.h"
#include "atca_device.h"
#include "atca_basic.h"
#include "atca_hal.h"
#include "calib_basic.h"
#include "calib_command.h"
#include "calib_execution.h"
#include "calib_aes_gcm.h"
#include "atca_crypto_sw.h"
#include "atca_crypto_sw_sha2.h"
#include "atca_crypto_hw_aes.h"
#include "types_basic.h"


#ifndef MYLIB_H_
#define MYLIB_H_
//
#define test_assert_config_is_locked()          atca_test_assert_config_is_locked(__LINE__)
#define test_assert_data_is_locked()            atca_test_assert_data_is_locked(__LINE__)
#define check_config_aes_enable()               atca_test_assert_aes_enabled(__LINE__)
#define test_assert_data_is_unlocked()     atca_test_assert_data_is_unlocked(__LINE__)


#define TEST_TYPE_ECC_SIGN          (1)
#define TEST_TYPE_ECC_VERIFY        (2)
#define TEST_TYPE_ECC_GENKEY        (3)
#define TEST_TYPE_AES               (4)
#define TEST_TYPE_DATA              (5)
#define TEST_TYPE_HMAC              (6)
#define TEST_TYPE_ECDH              (7)
#define TEST_TYPE_AUTH_HMAC         (8)
#define TEST_TYPE_AUTH_CMAC         (9)
#define TEST_TYPE_AUTH_GCM          (10)
#define TEST_TYPE_ECC_ROOT_KEY      (11)
#define TEST_TYPE_TEMPLATE_DATA     (12)

#define AES_CONFIG_ENABLE_BIT_MASK   (uint8_t)0x01

typedef unsigned int    UNITY_UINT32;
typedef UNITY_UINT32 UNITY_UINT;
#define UNITY_LINE_TYPE UNITY_UINT

typedef struct
{
    uint8_t  test_type;
    uint16_t handle;
    void*    attributes;
} device_object_meta_t;


extern const uint8_t g_slot4_key[];
//extern ATCAIfaceCfg *gCfg;

int factorial(int input);

ATCA_STATUS test_ecdh_protection_key(ATCAIfaceCfg* cfg);
ATCA_STATUS test_ecdh(ATCAIfaceCfg* cfg);

ATCA_STATUS inject_cert(int CERTIFICATE_KEYID/* 1 or 2*/, const uint8_t *cert, int cert_size);
ATCA_STATUS read_cert(int CERTIFICATE_KEYID/* 1 or 2*/, const uint8_t *cert, int *cert_size);
ATCA_STATUS get_random(uint8_t *rndData);
ATCA_STATUS gen_key(uint8_t *pubKey, uint8_t *keyID);
ATCA_STATUS get_key(uint8_t *pubKey, uint8_t *keyID);
ATCA_STATUS asymmetric_sign(uint16_t *private_key_id, uint8_t *msg, uint8_t *signature);
ATCA_STATUS asymmetric_verify(uint16_t *key_id, uint8_t *msg, uint8_t *signature, bool* is_verified);
ATCA_STATUS sha_digest(uint8_t *msg, uint8_t *digest, uint16_t *msg_size);
void sha_s_digest(uint8_t *msg, uint8_t *digest, uint16_t *msg_size);
ATCA_STATUS gen_aes_key(uint8_t *keyID);
ATCA_STATUS get_aes_key(uint8_t *aesKey, uint8_t *keyID, uint8_t *key_block);
ATCA_STATUS aes_encrypt_slot(uint8_t *key_slot, uint8_t *key_block, uint8_t *msg, size_t *msg_size, uint8_t *encrypted_data_out);
ATCA_STATUS aes_decrypt_slot(uint8_t *key_slot, uint8_t *key_block, uint8_t *ciphertext, size_t *ciphertext_size, uint8_t *decrypted_data_out);

ATCA_STATUS check_config_is_locked(void);
ATCA_STATUS check_data_is_locked(void);
ATCA_STATUS check_aes_enabled(void);





void init_Device(ATCAIface atecc, uint8_t address, int retries);
ATCA_STATUS hal_i2c_wake(ATCAIface iface);

ATCA_STATUS calib_config_get_slot_by_test(uint8_t test_type, uint16_t* handle);

#endif /* MYLIB_H_ */
