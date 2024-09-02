/******************************************************************************
 * Name      : S3A7_REANIBEX_100                                              *
 * Copyright : Osatu S. Coop                                                  *
 * Compiler  : MinGW32                                                        *
 * Target    : Reanibex Series                                                *
 ******************************************************************************/

/*!
 * @file        secure_crypto.c
 * @brief       NetX secure for check certificates.
 *              To use these functions you must first call nx_init().
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
#include "secure_crypto.h"
#include "synergy_gen/common_data.h"
#include "../synergy/ssp/inc/framework/el/nxd/nx_api.h"
#include "../synergy/ssp/inc/framework/el/cm23_iar/tx_port.h"
#include "../synergy/ssp/src/framework/el/nxd_application_layer/nxd_tls_secure/nx_secure_x509.h"
#include "../synergy/ssp/src/framework/el/nxd_application_layer/nxd_tls_secure/nx_secure_tls_api.h"

#include "SCEF/sha2.h"
#include "SCEF/aes.h"
#include "common_data.h"

/******************************************************************************
 ** Defines
 */

/******************************************************************************
 ** Typedefs
 */

/******************************************************************************
 ** Constants
 */
#define ASCII_TO_DEC(x)                 (uint8_t) ((x >= '0') ? (x-'0') : 0)

/******************************************************************************
 ** Externals
 */
WIFI_HEADER_t wifi_header;

/******************************************************************************
 ** Locals
 */
static FX_FILE     secure_file;

/******************************************************************************
 ** Globals
 */
atca_iface_t atecc_o;
ATCAIfaceCfg _mIfaceCFG;    ///< Points to previous defined/given Cfg object, the caller manages this
ATCAHAL_t    _hal;          ///< The configured HAL for the interface
ATCAHAL_t    _phy;          ///< When a HAL is not a "native" hal it needs a physical layer to be associated with it


static bool_t   atecc_initialized    = FALSE;
/******************************************************************************
 ** Prototypes
 */

/**
 * @brief   This function conpares certificate validation data with UTC-0 data.
 *          If the certificate has expired or is going to expire on the same day, its return an error
 *
 * @param[in] time_asn1 Certificate data in ASN1 format
 * @param[in] length Certificate length
 *
 * @return #CERT_VALID
 * @return #CERT_NOT_VALID
 */
static bool_t certificate_time(const UCHAR *time_asn1, uint16_t length)
{
    DEVICE_DATE_t *pDate;
    //DEVICE_TIME_t *pTime;

    uint8_t inc_month = 0;
    uint32_t day = 0, month = 0, year = 0;
    //uint32_t sec = 0, min = 0, hour = 0;
    uint32_t    day_1 = 0, day_2 = 0;
    //uint32_t    sec_1, sec_2;
    char_t c_year[2] = {0};
    char_t c_month[2] = {0};
    char_t c_day[2] = {0};

    /*char_t c_hour[2] = {0};
    char_t c_min[2] = {0};
    char_t c_sec[2] = {0};*/

    // read the power on date and time ...
    pDate = Get_Device_Date();
    //pTime = Get_Device_Time();

    // ASN1 time format: aaMMddHHmmssZ. The last "Z" (capital) sign means invariant zone (UTC or GMT 0) and MUST be placed at the end of the string.
    memcpy(c_year, time_asn1, length);
    year = (uint32_t)(((ASCII_TO_DEC(c_year[0])*10) + ASCII_TO_DEC(c_year[1]))+2000);

    memcpy(c_month, time_asn1+2, length);
    month = (uint32_t)((ASCII_TO_DEC(c_month[0])*10) + ASCII_TO_DEC(c_month[1]));

    memcpy(c_day, time_asn1+4, length);
    day = (uint32_t)((ASCII_TO_DEC(c_day[0])*10) + ASCII_TO_DEC(c_day[1]));

    /*memcpy(c_hour, time_asn1+6, length);
    hour = (uint32_t)((ASCII_TO_DEC(c_hour[0])*10) + ASCII_TO_DEC(c_hour[1]));

    memcpy(c_min, time_asn1+8, length);
    min = (uint32_t)((ASCII_TO_DEC(c_min[0])*10) + ASCII_TO_DEC(c_min[1]));

    memcpy(c_sec, time_asn1+10, length);
    sec = (uint32_t)((ASCII_TO_DEC(c_sec[0])*10) + ASCII_TO_DEC(c_sec[1]));*/

    // day_1: certificate time // day_2: UTC-0
    day_1 = (year  * 365) + ((month  & 0xFF) * 30) + (day  & 0xFF);
    day_2 = ((uint32_t)(pDate->year+2000) * 365) + (((uint32_t)pDate->month & 0xFF) * 30) + ((uint32_t)pDate->date  & 0xFF);

    if (day_1 >= day_2)
    {
        if(pDate->month == month && ((uint32_t)pDate->year+2000) == year)
        {
            return CERT_NOT_VALID; // The certificate will expire in one month.
        }
        inc_month = pDate->month;
        inc_month = (uint8_t)(inc_month + 2);
        if(inc_month == month && ((uint32_t)pDate->year+2000) == year)
        {
            return CERT_EXP_SOON; // The certificate will expire soon
        }

        return CERT_VALID;
    }
    /*if(day_1 > day_2)
    {
        sec_1 = ((hour >> 16) * 60 * 60) + (((min >> 8) & 0xFF) * 60) + (sec  & 0xFF);
        sec_2 = ((pTime->hour >> 16) * 60 * 60) + (((pTime->hour >> 8) & 0xFF) * 60) + (pTime->hour  & 0xFF);
        return (sec_1 - sec_2 > 0) ? 0 : 1
    }*/
    return CERT_NOT_VALID;
}

/**
 * @brief This function checks which certificates it has to check.
 *
 * @param eap Indicates whether the EAP certificate needs to be checked
 * @param tls Indicates whether the TLS certificate needs to be checked
 * 
 * @return none
 */
void Check_Certs(bool_t* eap, bool_t* tls)
{
    uint8_t  fx_res;
    uint32_t nBytes = 0;
    char_t   data[24];

    fx_res = (uint8_t) fx_directory_default_set(&sd_fx_media, CERT_FOLDER_NAME);
    if (fx_res == FX_SUCCESS)
    {
        // Load and read Test file on SD
        fx_res = (uint8_t) fx_file_open(&sd_fx_media, &secure_file, CERT_FILENAME, FX_OPEN_FOR_READ);
        if (fx_res == 0)
        {
            fx_res = (uint8_t) fx_file_read(&secure_file, data, 24, &nBytes);     //Read 100 bytes
            if (fx_res == 0)
            {
                if((uint8_t)(data[9] - '0') == 1) (*eap)++;
                if((uint8_t)(data[21] - '0') == 1) (*tls)++;
            }
        }
        fx_res = (uint8_t) fx_directory_default_set(&sd_fx_media, FX_NULL);
    }
}

/**
 * @brief   This function initialize NetX system
 *
 * @param void
 *
 * @return none
 */

void NX_Init(void)
{
    // Initialize the NetX system.
    nx_system_initialize();
}

/**
 * @brief This function read TLS CA certificate from SD and check the validity of the certificate.
 *        The certificate must be in DER format. DER is a binary encoding for X.509 certificate.
 *
 * @param void
 *
 * @return #CERT_VALID
 * @return #CERT_NOT_VALID
 */

uint16_t Check_TLS_Cacert(void)
{
    NX_SECURE_X509_CERT ca_certificate;
    //FX_FILE  eap_cert_file;

    uint8_t  fx_res;
    bool_t   ret_val;

    uint32_t read_nBytes = 0;
    uchar_t ca_cert_data[DER_Size_Limit] = {0};
    uint16_t status = 0;

    fx_res = (uint8_t) fx_directory_default_set(&sd_fx_media, CERT_FOLDER_NAME);
    if (fx_res == FX_SUCCESS)
    {
        // Hidden file
        //fx_res = (uint8_t) fx_file_attributes_set(&sd_fx_media, TLS_CA_CERT_FILENAME, FX_HIDDEN);

        // Load and read certificate file on SD
        fx_res = (uint8_t) fx_file_open(&sd_fx_media, &secure_file, TLS_CA_CERT_FILENAME, FX_OPEN_FOR_READ);
        if (fx_res == 0 && secure_file.fx_file_current_file_size < DER_Size_Limit)
        {
            fx_res = (uint8_t) fx_file_read(&secure_file, ca_cert_data, (ULONG)secure_file.fx_file_current_file_size, &read_nBytes);
            // Close the file
            fx_res = (uint8_t) fx_file_close(&secure_file);
        }
        else
        {
            if(fx_res == 0) fx_res = (uint8_t) fx_file_close(&secure_file);
            fx_res = (uint8_t) fx_directory_default_set(&sd_fx_media, FX_NULL);
            return eERR_NONE;
        }

        // Initialize an X.509 Certificate for NetX Secure TLS
        status = (uint16_t) nx_secure_x509_certificate_initialize(&ca_certificate, ca_cert_data, (USHORT)(secure_file.fx_file_current_file_size), NX_NULL, 0, NULL, 0, (UINT)NX_SECURE_X509_KEY_TYPE_NONE);
        if(status != 0)
        {
            fx_res = (uint8_t) fx_directory_default_set(&sd_fx_media, FX_NULL);
            return CERT_NOT_VALID;
        }

        // Reset to default directory
        fx_res = (uint8_t) fx_directory_default_set(&sd_fx_media, FX_NULL);

        if(status == 0x00)
        {
            ret_val = certificate_time(ca_certificate.nx_secure_x509_not_after, 2);
            if(ret_val == 0x00)
            {
                return CERT_VALID;
            }
            else if(ret_val == 0x01)
            {
                return CERT_EXP_SOON;
            }
            else return CERT_NOT_VALID;
        }
    }
    return eERR_NONE;
}

/**
 * @brief This function read WPA-EAP certificate from SD and check the validity of the certificate.
 *        The certificate must be in DER format. DER is a binary encoding for X.509 certificate.
 *
 * @param void
 *
 * @return #CERT_VALID
 * @return #CERT_NOT_VALID
 */

uint16_t Check_WPA_EAP_Cert(void)
{
    NX_SECURE_X509_CERT certificate;
    //FX_FILE  tls_cert_file;

    uint8_t  fx_res;
    bool_t   ret_val;

    uint32_t read_nBytes = 0;
    uchar_t cert_data[DER_Size_Limit] = {0};
    uint16_t status = 0;

    fx_res = (uint8_t) fx_directory_default_set(&sd_fx_media, CERT_FOLDER_NAME); // Cert directory
    if (fx_res == FX_SUCCESS)
    {
        // Hidden file
        //fx_res = (uint8_t) fx_file_attributes_set(sd_media, WPA_EAP_CA_CERT_FILENAME, FX_HIDDEN);

        // Load and read certificate file on SD
        fx_res = (uint8_t) fx_file_open(&sd_fx_media, &secure_file, WPA_EAP_CA_CERT_FILENAME, FX_OPEN_FOR_READ);
        if (fx_res == 0 && secure_file.fx_file_current_file_size < DER_Size_Limit)
        {
            fx_res = (uint8_t) fx_file_read(&secure_file, cert_data, (ULONG)secure_file.fx_file_current_file_size, &read_nBytes);
            // Close the file
            fx_res = (uint8_t) fx_file_close(&secure_file);
        }
        else
        {
            if(fx_res == 0) fx_res = (uint8_t) fx_file_close(&secure_file);
            fx_res = (uint8_t) fx_directory_default_set(&sd_fx_media, FX_NULL);
            return eERR_NONE;
        }

        // Initialize an X.509 Certificate for NetX Secure TLS
        status = (uint16_t) nx_secure_x509_certificate_initialize(&certificate, cert_data, (USHORT)(secure_file.fx_file_current_file_size), NX_NULL, 0, NULL, 0, (UINT)NX_SECURE_X509_KEY_TYPE_NONE);
        if(status != 0)
        {
            fx_res = (uint8_t) fx_directory_default_set(&sd_fx_media, FX_NULL);
            return CERT_NOT_VALID;
        }

        // Reset to default directory
        fx_res = (uint8_t) fx_directory_default_set(&sd_fx_media, FX_NULL);

        if(status == 0x00)
        {
            ret_val = certificate_time(certificate.nx_secure_x509_not_after, 2);
            if(ret_val == 0X00)
            {
                return CERT_VALID;
            }
            else if(ret_val == 0x01)
            {
                return CERT_EXP_SOON;
            }
            else return CERT_NOT_VALID;
        }
    }
    return eERR_NONE;
}

/******************************************************************************
 *                                                                            *
 *                      Secure Element ATECC608B-SSHDA-T                      *
 *                                                                            *
 *****************************************************************************/

/******************************************************************************
 ** Name:    Crypto_ATECC_Init
 *****************************************************************************/
/**
 ** @brief   Function to initialize the ATECC608B secure element
 **
 ** @return  ATCA_STATUS
 *****************************************************************************/
ATCA_STATUS Crypto_ATECC_Init(void)
{
    uint8_t address= 0xC0;       // I2C address TRUST CUSTOM ATECC608B-SSHDA-T
    int retries= 10;             // 10 result in 10 succsessful reads
    //uint8_t revision[10];
    ATCA_STATUS status = ATCA_NOT_INITIALIZED;

    // WARNING --> Be sure that the operation can be executed with the device powered
    // Power on the external circuits, adding an extra time to stabilize the power supplies ...
    g_ioport.p_api->pinWrite (IO_PSU_EN,  IOPORT_LEVEL_HIGH);
    tx_thread_sleep (OSTIME_20MSEC);

    atecc_o.mIfaceCFG = &_mIfaceCFG;
    atecc_o.phy = &_phy;
    ATCAIface atecc = &atecc_o;

    // Initialize the secure element
    init_Device(atecc, address, retries);

    //Wake up CryptoAuth device using I2C bus
    status = hal_i2c_wake(atecc);
    if(status == ATCA_SUCCESS)
    {
        ///////////////////////////Initialize///////////////////////////
        status = atcab_init(atecc->mIfaceCFG);
    }
    else
    {
        address= 0x6C; // I2C address TRUST FLEX ATECC608B-TFLXTLS
        // Initialize the secure element
        init_Device(atecc, address, retries);

        //Wake up CryptoAuth device using I2C bus
        status = hal_i2c_wake(atecc);
        if(status == ATCA_SUCCESS)
        {
            ///////////////////////////Initialize///////////////////////////
            status = atcab_init(atecc->mIfaceCFG);
        }
    }

/*
    status = check_config_is_locked();
    status = check_data_is_locked();
    status = check_aes_enabled();

    // get revision info
    //memset(revision, 0, 10);
    //status = get_info(revision, atecc->mIfaceCFG);
*/

    return status;
}

/******************************************************************************
 ** Name:    Crypto_ATECC_Config
 *****************************************************************************/
/**
 ** @brief   Function to configure and lock data and config zone.
 **
 ** @return  ATCA_STATUS
 *****************************************************************************/
ATCA_STATUS Crypto_ATECC_Config(void)
{
    ATCA_STATUS status = ATCA_NOT_INITIALIZED;
    //bool same_config = false;
    bool is_locked_config = false;
    bool is_locked_data = false;
    uint8_t pubKey[ATCA_ECCP256_PUBKEY_SIZE];
    uint8_t kID = 2;
    uint8_t ecc608_configdata_cmp[112];

    uint8_t ecc608_configdata[] = {
        /*0x01, 0x23, 0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 0x04, 0x05, 0x06, 0x07, 0xEE, 0x01, 0x01, 0x00,*/  //15  0-15 NEVER WRITE
        0xC0, 0x00, 0x00, 0x01, 0x85, 0x00, 0x82, 0x00, 0x85, 0x20, 0x85, 0x20, 0x85, 0x20, 0x8F, 0x46,  //31 slot 5
        0x8F, 0x0F, 0x9F, 0x8F, 0x0F, 0x0F, 0x8F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,  //47
        0x0D, 0x1F, 0x0F, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,  //63
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xF7, 0x00, 0x69, 0x76, 0x00, 0x00, 0x00, 0x00, 0x00,  //79
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0x55, 0xFF, 0xFF, 0x0E, 0x60, 0x00, 0x00, 0x00, 0x00,  //95
        0x53, 0x00, 0x53, 0x00, 0x73, 0x00, 0x73, 0x00, 0x73, 0x00, 0x38, 0x00, 0x7C, 0x00, 0x1C, 0x00,  //111
        0x3C, 0x00, 0x1A, 0x00, 0x3C, 0x00, 0x30, 0x00, 0x3C, 0x00, 0x30, 0x00, 0x12, 0x00, 0x30, 0x00   //127
    };
    /* // CONFIG IKERLAN
    uint8_t ecc608_configdata[] ={
        0xC0, 0x00, 0x00, 0x01, 0x85, 0x00, 0x82, 0x00, 0x85, 0x20, 0x85, 0x20, 0x85, 0x20, 0x8F, 0x46, //31
        0x8F, 0x0F, 0x9F, 0x0F, 0x0F, 0x8F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, //47
        0x0D, 0x1F, 0x0F, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, //63
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xF7, 0x00, 0x69, 0x76, 0x00, 0x00, 0x00, 0x00, 0x00, //79
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0x55, 0xFF, 0xFF, 0x0E, 0x60, 0x00, 0x00, 0x00, 0x00, //95
        0x53, 0x00, 0x53, 0x00, 0x73, 0x00, 0x73, 0x00, 0x73, 0x00, 0x38, 0x00, 0x7C, 0x00, 0x1C, 0x00, //111
        0x3C, 0x00, 0x1A, 0x00, 0x3C, 0x00, 0x30, 0x00, 0x3C, 0x00, 0x30, 0x00, 0x12, 0x00, 0x30, 0x00  //127
    };
    */

    status = Crypto_ATECC_Init();
    if(status == ATCA_SUCCESS)
    {
        //status = atcab_write_config_zone(config_zone);
        //status = atcab_read_config_zone(config_zone);
        status = atcab_is_config_locked(&is_locked_config);
        status = atcab_is_data_locked(&is_locked_data);
        if(is_locked_config == true && is_locked_data == true) return ATCA_SUCCESS;

        status = atcab_write_bytes_zone(ATCA_ZONE_CONFIG, 0, 16, ecc608_configdata, sizeof(ecc608_configdata));
        status = atcab_read_bytes_zone(ATCA_ZONE_CONFIG, 0, 16, ecc608_configdata_cmp, sizeof(ecc608_configdata_cmp));
        if((memcmp(ecc608_configdata, ecc608_configdata_cmp, sizeof(ecc608_configdata))) == 0)
        //status = atcab_cmp_config_zone(ecc608_configdata, &same_config); //ecc608_configdata tiene que ser de 128 bytes
        //if(same_config == true && status == ATCA_SUCCESS)
        {
            status = atcab_is_config_locked(&is_locked_config);  // Check if Config zone is locked
            if(status == ATCA_SUCCESS)
            {
                if(is_locked_config == false)
                {
                    status = atcab_lock_config_zone();  // Lock Config zone
                    if(status != ATCA_SUCCESS) return status;
                }
                status = atcab_is_data_locked(&is_locked_data);  // Check if Data zone is locked
                if(status == ATCA_SUCCESS)
                {
                    if(is_locked_data == false)
                    {
                        status = gen_key(pubKey, &kID);  // Generate key pairs
                        if(status == ATCA_SUCCESS)
                        {
                            status = atcab_lock_data_zone();  // Lock Data zone 
                            if(status != ATCA_SUCCESS) return status;
                        }
                    }
                    //status = atcab_is_slot_locked(aes_kID, &is_locked);
                    //status = atcab_lock_data_slot(aes_kID);
                }
            }
            if(status = Crypto_ATECC_Inject_Zero(), status != ATCA_SUCCESS) return status;
        }
        else return eRR_CRYPTO_AUTH;
    }
    return status;
}

/******************************************************************************
 ** Name:    Crypto_ATECC_IsPresent
 *****************************************************************************/
/**
 ** @brief   Function to check if ATECC is present
 **
 ** @return  TRUE if is Present/ FALSE not present
 *****************************************************************************/
bool_t Crypto_ATECC_IsPresent(void)
{
    ATCA_STATUS status = ATCA_NOT_INITIALIZED;

    status = Crypto_ATECC_Init();
    if(status == ATCA_SUCCESS)
    {
        atecc_initialized = TRUE;
        return TRUE;
    }
    else return FALSE;
}

/******************************************************************************
 ** Name:    Is_ATECC_Initialized
 *****************************************************************************/
/**
** @brief   Checks if atecc initialization process completed
**
** @return  TRUE if atecc initialized and FALSE if not
 *****************************************************************************/
bool_t Is_ATECC_Initialized(void)
{
    return atecc_initialized;
}

/******************************************************************************
 ** Name:    Crypto_ATECC_Encrypt
 *****************************************************************************/
/**
 ** @brief  Function to encrypt file
 **
 ** @param  r_w_data data to be encrypted
 **
 ** @return  ATCA_STATUS
 *****************************************************************************/
ATCA_STATUS Crypto_ATECC_Encrypt_NFC(uint8_t *r_w_data)
{
    uint8_t     fx_res;
    uint32_t    attributes;
    ATCA_STATUS status = ATCA_SUCCESS;

    uint8_t aes_kID = 9;    // Number of slot
    uint8_t aes_block= 0;   // Block ID
    //uint8_t r_w_data[CRYPTO_SIZE] = {0};
    uint8_t encrypted_data_out[WIFI_SIZE] = {0};

    // ... check if the requested test file exist. If so, delete it
    fx_res = (uint8_t) fx_file_attributes_read (&sd_fx_media, WIFI_FILENAME, (UINT *)&attributes);
    if ((fx_res == FX_SUCCESS) || (attributes & FX_ARCHIVE))
    {
        fx_res = (uint8_t) fx_file_delete(&sd_fx_media, WIFI_FILENAME);
        fx_res = (uint8_t) fx_media_flush (&sd_fx_media);
    }

    fx_res = (uint8_t) fx_file_create(&sd_fx_media, WIFI_FILENAME);
    if (fx_res == FX_SUCCESS)
    {
        status = Crypto_ATECC_Init();
        if (status == ATCA_SUCCESS)
        {
            status = gen_aes_key(&aes_kID); // Generate 4 AES keys in slot 9

            ///////////////////////////ATEC AES///////////////////////////
            status = aes_encrypt_slot(&aes_kID, &aes_block, r_w_data, WIFI_SIZE, encrypted_data_out);

            fx_res = (uint8_t) fx_file_attributes_set(&sd_fx_media, WIFI_FILENAME, FX_HIDDEN);
            fx_res = (uint8_t) fx_file_open(&sd_fx_media, &secure_file, WIFI_FILENAME, FX_OPEN_FOR_WRITE);
            fx_res = (uint8_t) fx_file_seek(&secure_file, 0);
            fx_res = (uint8_t) fx_file_write(&secure_file, encrypted_data_out, WIFI_SIZE);
            fx_res = (uint8_t) fx_media_flush (&sd_fx_media);
            fx_res = (uint8_t) fx_file_close(&secure_file);

            if(status != ATCA_SUCCESS)
            {
                // If the module does not respond, for security reasons delete the credentials from the file
                fx_res = (uint8_t) fx_file_delete(&sd_fx_media, WIFI_FILENAME);
                fx_res = (uint8_t) fx_media_flush (&sd_fx_media);
                /*memset(r_w_data, 0, WIFI_SIZE);
                fx_res = (uint8_t) fx_file_open(&sd_fx_media, &secure_file, WIFI_FILENAME, FX_OPEN_FOR_WRITE);
                fx_res = (uint8_t) fx_file_seek(&secure_file, 0);
                fx_res = (uint8_t) fx_file_write(&secure_file, r_w_data, WIFI_SIZE);
                fx_res = (uint8_t) fx_media_flush (&sd_fx_media);
                fx_res = (uint8_t) fx_file_close(&secure_file);*/
            }
        }
    }
    if(status != ATCA_SUCCESS) return eRR_CRYPTO_AUTH;

    return eERR_NONE;
}

/******************************************************************************
 ** Name:    Crypto_ATECC_Encrypt
 *****************************************************************************/
/**
 ** @brief   Function to encrypt file
 **
 ** @return  ATCA_STATUS
 *****************************************************************************/
ATCA_STATUS Crypto_ATECC_Encrypt_File(void)
{
    uint8_t     fx_res;
    ATCA_STATUS status = ATCA_SUCCESS;

    uint8_t aes_kID = 9;    // Number of slot
    uint8_t aes_block= 0;   // Block ID
    uint32_t nBytes, attributes;
    uint8_t r_w_data[WIFI_SIZE] = {0};
    uint8_t encrypted_data_out[WIFI_SIZE] = {0};

    // ... check if the requested test file exist. If so, delete it
    fx_res = (uint8_t) fx_file_attributes_read (&sd_fx_media, WIFI_FILENAME_PLAIN, (UINT *)&attributes);
    if ((fx_res == FX_SUCCESS) || (attributes & FX_ARCHIVE))
    {
        fx_res = (uint8_t) fx_file_delete(&sd_fx_media, WIFI_FILENAME);
        fx_res = (uint8_t) fx_media_flush (&sd_fx_media);
    }
    else return eERR_NONE;

    fx_res = (uint8_t) fx_file_attributes_set(&sd_fx_media, WIFI_FILENAME, FX_HIDDEN);
    fx_res = (uint8_t) fx_file_open(&sd_fx_media, &secure_file, WIFI_FILENAME_PLAIN, FX_OPEN_FOR_READ);
    if (fx_res == FX_SUCCESS)
    {
        fx_res = (uint8_t) fx_file_read(&secure_file, r_w_data, WIFI_SIZE, &nBytes);
        fx_res = (uint8_t) fx_media_flush (&sd_fx_media);
        fx_res = (uint8_t) fx_file_close(&secure_file);

        status = Crypto_ATECC_Init();
        if (status == ATCA_SUCCESS)
        {
            status = gen_aes_key(&aes_kID); // Generate 4 AES keys in slot 9

            ///////////////////////////ATEC AES///////////////////////////
            status = aes_encrypt_slot(&aes_kID, &aes_block, r_w_data, WIFI_SIZE, encrypted_data_out);

            fx_res = (uint8_t) fx_file_open(&sd_fx_media, &secure_file, WIFI_FILENAME_PLAIN, FX_OPEN_FOR_WRITE);
            fx_res = (uint8_t) fx_file_seek(&secure_file, 0);
            fx_res = (uint8_t) fx_file_write(&secure_file, encrypted_data_out, WIFI_SIZE);
            fx_res = (uint8_t) fx_media_flush (&sd_fx_media);
            fx_res = (uint8_t) fx_file_close(&secure_file);

            fx_res = (uint8_t) fx_file_rename(&sd_fx_media, WIFI_FILENAME_PLAIN, WIFI_FILENAME);

            if(status != ATCA_SUCCESS)
            {
                // If the module does not respond, for security reasons delete the credentials from the file
                fx_res = (uint8_t) fx_file_delete(&sd_fx_media, WIFI_FILENAME);
                fx_res = (uint8_t) fx_media_flush (&sd_fx_media);
                /*memset(r_w_data, 0, WIFI_SIZE);
                fx_res = (uint8_t) fx_file_open(&sd_fx_media, &secure_file, WIFI_FILENAME, FX_OPEN_FOR_WRITE);
                fx_res = (uint8_t) fx_file_seek(&secure_file, 0);
                fx_res = (uint8_t) fx_file_write(&secure_file, r_w_data, WIFI_SIZEs);
                fx_res = (uint8_t) fx_media_flush (&sd_fx_media);
                fx_res = (uint8_t) fx_file_close(&secure_file);*/
            }
        }
    }
    if(status != ATCA_SUCCESS) return eRR_CRYPTO_AUTH;

    return eERR_NONE;
}

/******************************************************************************
 ** Name:    Crypto_ATECC_Decrypt
 *****************************************************************************/
/**
 ** @brief   Function to encrypt data
 **
 ** @return  ATCA_STATUS
 *****************************************************************************/
ATCA_STATUS Crypto_ATECC_Decrypt(uint8_t *r_w_data)
{
    //uint8_t     fx_res;
    //uint32_t nBytes;
    ATCA_STATUS status = ATCA_NOT_INITIALIZED;

    uint8_t aes_kID = 9;    // Number of slot
    uint8_t aes_block= 0;   // Block ID
    uint8_t decrypted_data_out[WIFI_SIZE] = {0};
    //uint8_t r_w_data_aux[WIFI_SIZE] = {0};

    status = Crypto_ATECC_Init();
    if (status == ATCA_SUCCESS)
    {
        /*fx_res = (uint8_t) fx_file_open(&sd_fx_media, &secure_file, WIFI_FILENAME, FX_OPEN_FOR_READ);
        if (fx_res == FX_SUCCESS)
        {
            fx_res = (uint8_t) fx_file_read(&secure_file, r_w_data_aux, WIFI_SIZE, &nBytes);
            fx_res = (uint8_t) fx_media_flush (&sd_fx_media);
            fx_res = (uint8_t) fx_file_close(&secure_file);
        }*/

        status = aes_decrypt_slot(&aes_kID, &aes_block, r_w_data, WIFI_SIZE, decrypted_data_out);
        memcpy(r_w_data, decrypted_data_out, WIFI_SIZE);
    }
    if(status != ATCA_SUCCESS) return eRR_CRYPTO_AUTH;

    return eERR_NONE;
}

/******************************************************************************
 ** Name:    Crypto_ATECC_Decrypt_File
 *****************************************************************************/
/**
 ** @brief   Function to decrypt file
 **
 ** @return  ATCA_STATUS
 *****************************************************************************/
ATCA_STATUS Crypto_ATECC_Decrypt_File(void)
{
    uint8_t     fx_res;
    ATCA_STATUS status = ATCA_NOT_INITIALIZED;

    uint8_t aes_kID = 9;    // Number of slot
    uint8_t aes_block= 0;   // Block ID
    uint32_t nBytes;
    uint8_t r_w_data[CRYPTO_SIZE] = {0};
    uint8_t decrypted_data_out[CRYPTO_SIZE] = {0};

    status = Crypto_ATECC_Init();
    if (status == ATCA_SUCCESS)
    {
        fx_res = (uint8_t) fx_file_open(&sd_fx_media, &secure_file, WIFI_FILENAME, FX_OPEN_FOR_READ);
        if (fx_res == FX_SUCCESS)
        {
            fx_res = (uint8_t) fx_file_read(&secure_file, r_w_data, CRYPTO_SIZE, &nBytes);
            fx_res = (uint8_t) fx_media_flush (&sd_fx_media);
            fx_res = (uint8_t) fx_file_close(&secure_file);
        }

        ///////////////////////////ATEC AES///////////////////////////
        status = aes_decrypt_slot(&aes_kID, &aes_block, r_w_data, CRYPTO_SIZE, decrypted_data_out);

        fx_res = (uint8_t) fx_file_open(&sd_fx_media, &secure_file, WIFI_FILENAME, FX_OPEN_FOR_WRITE);
        fx_res = (uint8_t) fx_file_seek(&secure_file, 0);
        fx_res = (uint8_t) fx_file_write(&secure_file, decrypted_data_out, CRYPTO_SIZE);
        fx_res = (uint8_t) fx_media_flush (&sd_fx_media);
        fx_res = (uint8_t) fx_file_close(&secure_file);
    }
    return status;
}

/******************************************************************************
 ** Name:    Crypto_ATECC_Inject_Zero
 *****************************************************************************/
/**
 ** @brief   Function to inject in slot
 **
 ** @return  ATCA_STATUS
 *****************************************************************************/
ATCA_STATUS Crypto_ATECC_Inject_Zero(void)
{
    ATCA_STATUS status = ATCA_NOT_INITIALIZED;
    uint8_t inject_data[96] = {0};

    status = inject_cert(1, inject_data, sizeof(inject_data));

    return status;
}

/******************************************************************************
 ** Name:    Crypto_ATECC_Inject
 *****************************************************************************/
/**
 ** @brief   Function to inject public key and USB key in slot
 **
 ** @return  ATCA_STATUS
 *****************************************************************************/
ATCA_STATUS Crypto_ATECC_Inject(void)
{
    //FX_FILE  public_key_file;
    //FX_FILE  hash_file;
    //FX_FILE  signature_file;
    uint8_t     fx_res;
    ATCA_STATUS status = ATCA_NOT_INITIALIZED;

    uint32_t nBytes;
    uint8_t inject_data[96] = {0};
    //uint8_t read_data[96] = {0};
    //uint8_t certR[64];
    int certR_size = 0;

    status = Crypto_ATECC_Init();
    if (status == ATCA_SUCCESS)
    {
        status = read_cert(1, inject_data, &certR_size);
        //fx_res = (uint8_t) fx_directory_default_set(&sd_fx_media, CERT_FOLDER_NAME); // Reset default directory
        //if (fx_res == FX_SUCCESS)
        {
            fx_res = (uint8_t) fx_file_open(&sd_fx_media, &secure_file, PUBLIC_KEY, FX_OPEN_FOR_READ);
            if (fx_res == FX_SUCCESS)
            {
                fx_res = (uint8_t) fx_file_read(&secure_file, inject_data, 64, &nBytes);
                //status = inject_cert(1, inject_data, sizeof(inject_data));
                //status = read_cert(1, read_data, &certR_size);
                //memcpy(&inject_data[64], read_data, 32);
                //ret = memcmp(inject_data, certR, 64);
                fx_res = (uint8_t) fx_media_flush (&sd_fx_media);
                fx_res = (uint8_t) fx_file_close(&secure_file);
                fx_res = (uint8_t) fx_file_delete(&sd_fx_media, PUBLIC_KEY);
                fx_res = (uint8_t) fx_media_flush (&sd_fx_media);
            }
        }
        {
            fx_res = (uint8_t) fx_file_open(&sd_fx_media, &secure_file, USB_KEY, FX_OPEN_FOR_READ);
            if (fx_res == FX_SUCCESS)
            {
                fx_res = (uint8_t) fx_file_read(&secure_file, &inject_data[64], 32, &nBytes);
                fx_res = (uint8_t) fx_media_flush (&sd_fx_media);
                fx_res = (uint8_t) fx_file_close(&secure_file);
                fx_res = (uint8_t) fx_file_delete(&sd_fx_media, USB_KEY);
                fx_res = (uint8_t) fx_media_flush (&sd_fx_media);
            }
        }
        status = inject_cert(1, inject_data, sizeof(inject_data));
        //status = read_cert(1, certR, &certR_size);
        //fx_res = (uint8_t) fx_directory_default_set(&sd_fx_media, FX_NULL);
    }
    return status;
}

/******************************************************************************
 ** Name:    Crypto_ATECC_Verify
 *****************************************************************************/
/**
 ** @brief   Function to verify signature
 **
 ** @return  ATCA_STATUS
 *****************************************************************************/
ATCA_STATUS Crypto_ATECC_Verify(bool *is_verified)
{
    uint8_t     fx_res;
    UNUSED(fx_res);
    ATCA_STATUS status = ATCA_NOT_INITIALIZED;

    uint8_t public_key_data[64] = {0};  // The public key to be used for verification.
    uint8_t msg_hash[32] = {0};         // SHA256 Hash off receive data
    uint8_t signature[64] = {0};        // Signature to be verified.
    int pub_size;

    status = Crypto_ATECC_Init();
    if (status == ATCA_SUCCESS)
    {
        // Read public key stored in slot
        status = read_cert(1, public_key_data, &pub_size);
        if(status != ATCA_SUCCESS) return status;

        // Generate hash from receive firmware
        Crypto_ATECC_SHA256(msg_hash);

        // copy signature from server
        memcpy(signature, wifi_header.password, 64);

        status = atcab_verify_extern(msg_hash, signature, public_key_data, is_verified);
        if(status != ATCA_SUCCESS)
        {
            fx_res = (uint8_t) fx_file_delete(&sd_fx_media, UPGRADE_FILENAME);
            fx_res = (uint8_t) fx_media_flush (&sd_fx_media);
        }
    }
    return status;
}

/******************************************************************************
 ** Name:    Crypto_ATECC_Verify_Test
 *****************************************************************************/
/**
 ** @brief   Function to verify signature only for test
 **
 ** @return  ATCA_STATUS
 *****************************************************************************/
ATCA_STATUS Crypto_ATECC_Verify_Test(bool *is_verified)
{
    uint8_t     fx_res;
    ATCA_STATUS status = ATCA_NOT_INITIALIZED;

    uint32_t nBytes;
    uint8_t public_key_data[64] = {0};
    uint8_t msg_hash[32] = {0};
    uint8_t signature[64] = {0};
    int pub_size;

    status = Crypto_ATECC_Init();
    if (status == ATCA_SUCCESS)
    {
        // Read public key stored in slot
        status = read_cert(1, public_key_data, &pub_size);
        if(status != ATCA_SUCCESS) return status;

        fx_res = (uint8_t) fx_directory_default_set(&sd_fx_media, CERT_FOLDER_NAME); // Reset default directory
        if (fx_res == FX_SUCCESS)
        {
            fx_res = (uint8_t) fx_file_open(&sd_fx_media, &secure_file, SIGNATURE, FX_OPEN_FOR_READ);
            if (fx_res == FX_SUCCESS)
            {
                fx_res = (uint8_t) fx_file_read(&secure_file, signature, 64, &nBytes);
                fx_res = (uint8_t) fx_media_flush (&sd_fx_media);
                fx_res = (uint8_t) fx_file_close(&secure_file);
            }

            fx_res = (uint8_t) fx_file_open(&sd_fx_media, &secure_file, DIGEST, FX_OPEN_FOR_READ);
            if (fx_res == FX_SUCCESS)
            {
                fx_res = (uint8_t) fx_file_read(&secure_file, msg_hash, 32, &nBytes);
                fx_res = (uint8_t) fx_media_flush (&sd_fx_media);
                fx_res = (uint8_t) fx_file_close(&secure_file);
            }

            //Crypto_ATECC_SHA256(msg_hash);

            status = atcab_verify_extern(msg_hash, signature, public_key_data, is_verified);
            //status = asymmetric_verify(8, msg_hash, signature, &is_verified);

            //if(status == ATCA_SUCCESS)
            //{
            //    fx_res = (uint8_t) fx_file_delete(&sd_fx_media, "public_key.pem");
            //    fx_res = (uint8_t) fx_media_flush (&sd_fx_media);
            //}
        }
        fx_res = (uint8_t) fx_directory_default_set(&sd_fx_media, FX_NULL);
    }
    return status;
}

/******************************************************************************
 ** Name:    Crypto_ATECC_SHA256
 *****************************************************************************/
/**
 ** @brief   Function to generate SHA256
 **
 ** @param digest Array to save digest
 **
 ** @return  ATCA_STATUS
 *****************************************************************************/
ATCA_STATUS Crypto_ATECC_SHA256(uint8_t *digest)
{
    SHA256_CTX ctx;
    ATCA_STATUS status = ATCA_NOT_INITIALIZED;

    uint8_t     fx_res;
    uint32_t    nBytes = 0, h;
    uint64_t    file_size = 0;
    uint32_t    nFrame = 0;
    uchar_t     read_file_data[255]= {0};

    fx_res = (uint8_t) fx_directory_default_set(&sd_fx_media, NULL);
    if (fx_res == FX_SUCCESS)
    {
        fx_res = (uint8_t) fx_file_open(&sd_fx_media, &secure_file, UPGRADE_FILENAME, FX_OPEN_FOR_READ);
        if (fx_res == FX_SUCCESS)
        {
            memcpy(&file_size, &secure_file.fx_file_current_file_size, sizeof(uint64_t));
            // Check file length
            nFrame = (uint32_t) ((secure_file.fx_file_current_file_size) / 250 );
            if ((secure_file.fx_file_current_file_size) % 250 )
            {
                nFrame++;
            }

            //status = Crypto_ATECC_Init();
            //if (status == ATCA_SUCCESS)
            {
                sha256_init(&ctx);

                for(h=0; h < nFrame; h++)
                {
                    fx_res = (uint8_t) fx_file_read(&secure_file, read_file_data, 250, &nBytes);
                    sha256_update(&ctx, read_file_data, nBytes);
                }

                sha256_final(&ctx, digest);
            }
        }
        fx_res = (uint8_t) fx_file_close(&secure_file);
        fx_res = (uint8_t) fx_media_flush (&sd_fx_media);
    }

    return status;
}

/******************************************************************************
 ** Name:    Crypto_ATECC_SHA256
 *****************************************************************************/
/**
 ** @brief   Function to generate SHA256 HMAC
 **
 ** @param digest Array to save digest
 **
 ** @return  ATCA_STATUS
 *****************************************************************************/
ATCA_STATUS Crypto_ATECC_SHA256_HMAC(uint8_t *digest)
{
    atcac_hmac_sha256_ctx ctx;
    ATCA_STATUS status = ATCA_NOT_INITIALIZED;

    uint8_t     fx_res;
    uint32_t    nBytes = 0, h;
    uint64_t    file_size = 0;
    uint32_t    nFrame = 0;
    uchar_t     read_file_data[255]= {0};
    uint8_t     password=0, password_len=0;
    //size_t      digest_size = ATCA_SHA256_DIGEST_SIZE;

    fx_res = (uint8_t) fx_directory_default_set(&sd_fx_media, NULL);
    if (fx_res == FX_SUCCESS)
    {
        fx_res = (uint8_t) fx_file_open(&sd_fx_media, &secure_file, UPGRADE_FILENAME, FX_OPEN_FOR_READ);
        if (fx_res == FX_SUCCESS)
        {
            memcpy(&file_size, &secure_file.fx_file_current_file_size, sizeof(uint64_t));
            // Check file length
            nFrame = (uint32_t) ((secure_file.fx_file_current_file_size) / 250 );
            if ((secure_file.fx_file_current_file_size) % 250 )
            {
                nFrame++;
            }

            status = Crypto_ATECC_Init();
            if (status == ATCA_SUCCESS)
            {
                status = atcac_sha256_hmac_init(&ctx, &password, password_len);

                for(h=0; h < nFrame; h++)
                {
                    fx_res = (uint8_t) fx_file_read(&secure_file, read_file_data, 250, &nBytes);
                    status = atcac_sha256_hmac_update(&ctx, read_file_data, nBytes);
                }

                status = atcac_sha256_hmac_finish(&ctx, digest, ATCA_SHA256_DIGEST_SIZE);
            }
        }
        fx_res = (uint8_t) fx_file_close(&secure_file);
        fx_res = (uint8_t) fx_media_flush (&sd_fx_media);
    }

    return status;
}

/******************************************************************************
 *                                                                            *
 *                     Secure Cryptographic Engine (SCE5)                     *
 *                                                                            *
 *****************************************************************************/

/******************************************************************************
 ** Name:    Encrypt_Wifi_File
 *****************************************************************************/
/**
 ** @brief   Encrypt wifi file.
 **
 ** @return  none
 *****************************************************************************/
/*
void Encrypt_Wifi_File(void)
{
    uint8_t     fx_res;
    uint32_t    nBytes = 0, err;
    //uint32_t    cryptoStatus;
    uint8_t     r_w_data[200] = {0}, msg_enc[200];
    uint32_t    clave[4] = {0x768EDCB3, 0x3D2E98E6, 0xBC60F138, 0x3AE70142};
    uint32_t    init_enc[4] = {0x81A50AE3, 0x94EDBD12, 0x3BCBD20D, 0x892F7BF8};

    uint32_t  message_len     = (sizeof(r_w_data)) / (sizeof(r_w_data[0]));
    uint32_t new_message_len = (message_len * sizeof(char)) / sizeof(uint32_t);

    // En componentes activar r_sce y luego en Threads-> HAL/Common-> New stac-> driver-> crypto -> g_sce_aes
    //err = g_sce.p_api->open(g_sce.p_ctrl, g_sce.p_cfg);
    //err = g_sce.p_api->statusGet(&cryptoStatus);
    err = g_sce_aes_0.p_api->open(g_sce_aes_0.p_ctrl, g_sce_aes_0.p_cfg);

    // Load and read wifi.dat file on SD
    fx_res = (uint8_t) fx_file_open(&sd_fx_media, &secure_file, WIFI_FILENAME, FX_OPEN_FOR_READ);
    if (fx_res == FX_SUCCESS)
    {
        // Read data from file
        fx_res = (uint8_t) fx_file_read(&secure_file, r_w_data, 200, &nBytes);
        // Encrypt data
        err = g_sce_aes_0.p_api->encrypt( g_sce_aes_0.p_ctrl, clave, init_enc, new_message_len, (uint32_t*) r_w_data, (uint32_t*) msg_enc );

        fx_res = (uint8_t) fx_media_flush (&sd_fx_media);
        fx_res = (uint8_t) fx_file_close(&secure_file);

        if(err != SSP_SUCCESS)
        {
            // If the module does not respond, for security reasons delete the credentials from the file
            fx_res = (uint8_t) fx_file_delete(&sd_fx_media, WIFI_FILENAME);
            fx_res = (uint8_t) fx_media_flush (&sd_fx_media);
        }
        else
        {
            // Write encrypted data
            fx_res = (uint8_t) fx_file_open(&sd_fx_media, &secure_file, WIFI_FILENAME, FX_OPEN_FOR_WRITE);
            fx_res = (uint8_t) fx_file_seek(&secure_file, 0);
            fx_res = (uint8_t) fx_file_write(&secure_file, msg_enc, 200);
            fx_res = (uint8_t) fx_file_close(&secure_file);
            fx_res = (uint8_t) fx_media_flush (&sd_fx_media);
        }
    }

    err = g_sce_aes_0.p_api->close(g_sce_aes_0.p_ctrl);
}
*/
/******************************************************************************
 ** Name:    Decrypt_Wifi_File
 *****************************************************************************/
/**
 ** @brief   Decrypt wifi file and save data.
 **
 ** @return  none
 *****************************************************************************/
/*
void Decrypt_Wifi_File(void)
{
    uint8_t     fx_res;
    uint32_t    nBytes = 0, err;
    uint8_t     r_w_data[200] = {0}, msg_dec[200];
    uint32_t    clave[4] = {0x768EDCB3, 0x3D2E98E6, 0xBC60F138, 0x3AE70142};
    uint32_t    init_dec[4] = {0x81A50AE3, 0x94EDBD12, 0x3BCBD20D, 0x892F7BF8};

    uint32_t  message_len     = (sizeof(r_w_data)) / (sizeof(r_w_data[0]));
    uint32_t new_message_len = (message_len * sizeof(char)) / sizeof(uint32_t);

    err = g_sce_aes_0.p_api->open(g_sce_aes_0.p_ctrl, g_sce_aes_0.p_cfg);

    // Load and read certificate file on SD
    fx_res = (uint8_t) fx_file_open(&sd_fx_media, &secure_file, WIFI_FILENAME, FX_OPEN_FOR_READ);
    if (fx_res == FX_SUCCESS)
    {
        // Read data from file
        fx_res = (uint8_t) fx_file_read(&secure_file, r_w_data, 200, &nBytes);
        // Dencrypt data
        err = g_sce_aes_0.p_api->decrypt( g_sce_aes_0.p_ctrl, clave, init_dec, new_message_len, (uint32_t*) r_w_data, (uint32_t*) msg_dec );

        fx_res = (uint8_t) fx_media_flush (&sd_fx_media);
        fx_res = (uint8_t) fx_file_close(&secure_file);

        if(err != SSP_SUCCESS)
        {
            // If the module does not respond, for security reasons delete the credentials from the file
            fx_res = (uint8_t) fx_file_delete(&sd_fx_media, WIFI_FILENAME);
            fx_res = (uint8_t) fx_media_flush (&sd_fx_media);
        }
        else
        {
            nBytes = 0;
            for (int i = 0; i < 255; i++)
            {
                if(msg_dec[i] == '\0') break;
                nBytes++;
            }
            // Write decrypted data
            fx_res = (uint8_t) fx_file_delete(&sd_fx_media, WIFI_FILENAME);
            fx_res = (uint8_t) fx_file_create(&sd_fx_media, WIFI_FILENAME);
            fx_res = (uint8_t) fx_file_open(&sd_fx_media, &secure_file, WIFI_FILENAME, FX_OPEN_FOR_WRITE);
            fx_res = (uint8_t) fx_file_write(&secure_file, msg_dec, nBytes);
            fx_res = (uint8_t) fx_media_flush (&sd_fx_media);
            fx_res = (uint8_t) fx_file_close(&secure_file);
            fx_res = (uint8_t) fx_file_attributes_set(&sd_fx_media, WIFI_FILENAME, FX_HIDDEN);
        }
    }

    err = g_sce_aes_0.p_api->close(g_sce_aes_0.p_ctrl);
}
*/

/******************************************************************************
 *                                                                            *
 *                      Implementation of AES Algorithm                       *
 *                                                                            *
 *****************************************************************************/
/******************************************************************************
 ** Name:    AES_Algo_Decrypt
 *****************************************************************************/
/**
 ** @brief   Decrypt wifi file without saving data.
 **
 ** @return  none
 *****************************************************************************/
/*void AES_Algo_Decrypt(char_t* r_w_data)
{
    uint8_t i;
    uint8_t key[32] = {0};
    uint8_t iv[]  = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };

    fmi_unique_id_t unique_id;
    g_fmi.p_api->uniqueIdGet(&unique_id);
    memcpy(key, unique_id.unique_id, 16);

    for (i=0; i<16; i++)
    {
        key[i+16] = (uint8_t)(key[i+2] + key [i]);
    }

    // Decrypt wifi file
    struct AES_ctx ctx;
    AES_init_ctx_iv(&ctx, key, iv);
    AES_CBC_decrypt_buffer(&ctx, (uchar_t*)r_w_data, CRYPTO_SIZE);
}*/

/******************************************************************************
 ** Name:    AES_Algo_Encrypt_File
 *****************************************************************************/
/**
 ** @brief   Encrypt wifi file and save data.
 **
 ** @return  none
 *****************************************************************************/
/*void AES_Algo_Encrypt_File(void)
{
    uint8_t     fx_res, i;
    uint32_t    nBytes = 0;
    uchar_t     r_w_data[CRYPTO_SIZE];

    uint8_t key[32] = {0};
    uint8_t iv[]  = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };

    fmi_unique_id_t unique_id;
    g_fmi.p_api->uniqueIdGet(&unique_id);
    memcpy(key, unique_id.unique_id, 16);

    for (i=0; i<16; i++)
    {
        key[i+16] = (uint8_t)(key[i+2] + key [i]);
    }

    // Load and read wifi.dat file on SD
    fx_res = (uint8_t) fx_file_open(&sd_fx_media, &secure_file, WIFI_FILENAME, FX_OPEN_FOR_READ);
    if (fx_res == FX_SUCCESS)
    {
        // Read data from file
        fx_res = (uint8_t) fx_file_read(&secure_file, r_w_data, CRYPTO_SIZE, &nBytes);
        fx_res = (uint8_t) fx_media_flush (&sd_fx_media);
        fx_res = (uint8_t) fx_file_close(&secure_file);
    }

    struct AES_ctx ctx;
    AES_init_ctx_iv(&ctx, key, iv);
    AES_CBC_encrypt_buffer(&ctx, r_w_data, CRYPTO_SIZE);

    fx_res = (uint8_t) fx_file_open(&sd_fx_media, &secure_file, WIFI_FILENAME, FX_OPEN_FOR_WRITE);
    fx_res = (uint8_t) fx_file_seek(&secure_file, 0);
    fx_res = (uint8_t) fx_file_write(&secure_file, r_w_data, CRYPTO_SIZE);
    fx_res = (uint8_t) fx_file_close(&secure_file);
    fx_res = (uint8_t) fx_media_flush (&sd_fx_media);
}*/

/******************************************************************************
 ** Name:    AES_Algo_Decrypt_File
 *****************************************************************************/
/**
 ** @brief   Decrypt wifi file and save data.
 **
 ** @return  none
 *****************************************************************************/
/*void AES_Algo_Decrypt_File(void)
{
    uint8_t     fx_res, i;
    uint32_t    nBytes = 0;
    uchar_t     r_w_data[CRYPTO_SIZE];

    uint8_t key[32] = {0};
    uint8_t iv[]  = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };

    fmi_unique_id_t unique_id;
    g_fmi.p_api->uniqueIdGet(&unique_id);
    memcpy(key, unique_id.unique_id, 16);

    for (i=0; i<16; i++)
    {
        key[i+16] = (uint8_t)(key[i+2] + key [i]);
    }

    // Load and read wifi.dat file on SD
    fx_res = (uint8_t) fx_file_open(&sd_fx_media, &secure_file, WIFI_FILENAME, FX_OPEN_FOR_READ);
    if (fx_res == FX_SUCCESS)
    {
        // Read data from file
        fx_res = (uint8_t) fx_file_read(&secure_file, r_w_data, CRYPTO_SIZE, &nBytes);
        fx_res = (uint8_t) fx_media_flush (&sd_fx_media);
        fx_res = (uint8_t) fx_file_close(&secure_file);
    }

    struct AES_ctx ctx;
    AES_init_ctx_iv(&ctx, key, iv);
    AES_CBC_decrypt_buffer(&ctx, r_w_data, CRYPTO_SIZE);

    fx_res = (uint8_t) fx_file_open(&sd_fx_media, &secure_file, WIFI_FILENAME, FX_OPEN_FOR_WRITE);
    fx_res = (uint8_t) fx_file_seek(&secure_file, 0);
    fx_res = (uint8_t) fx_file_write(&secure_file, r_w_data, CRYPTO_SIZE);
    fx_res = (uint8_t) fx_file_close(&secure_file);
    fx_res = (uint8_t) fx_media_flush (&sd_fx_media);
}*/

/******************************************************************************
 ** Name:    SHA2_SHA256
 *****************************************************************************/
/**
 ** @brief   Generate SREC file SHA-256
 **
 ** @param data_hex Array to save SHA256
 **
 ** @return  none
 *****************************************************************************/
/*void SHA2_SHA256(char_t* data_hex)
{
    unsigned char sha2sum[32];
    sha2_context ctx;

    uint8_t     fx_res, i;
    uint32_t    nBytes = 0, h;
    uint64_t    file_size = 0;
    uint32_t    nFrame = 0;
    uchar_t     read_file_data[255]= {0};

    fx_res = (uint8_t) fx_directory_default_set(&sd_fx_media, NULL);
    fx_res = (uint8_t) fx_file_open(&sd_fx_media, &secure_file, UPGRADE_FILENAME, FX_OPEN_FOR_READ);
    if (fx_res == FX_SUCCESS)
    {
        memcpy(&file_size, &secure_file.fx_file_current_file_size, sizeof(uint64_t));
        // Check file length
        nFrame = (uint32_t) ((secure_file.fx_file_current_file_size) / 250 );
        if ((secure_file.fx_file_current_file_size) % 250 )
        {
            nFrame++;
        }
        sha2_starts(&ctx, 0);
        // Calculate file digest
        for(h=0; h < nFrame; h++)
        {
            fx_res = (uint8_t) fx_file_read(&secure_file, read_file_data, 250, &nBytes);
            sha2_update(&ctx, read_file_data, nBytes);
        }
        sha2_finish(&ctx, sha2sum);

    }
    fx_res = (uint8_t) fx_file_close(&secure_file);
    fx_res = (uint8_t) fx_media_flush (&sd_fx_media);

    for (i = 0 ; i != 32 ; i++)
    {
        //lint -e586 Admissible use of sprintf (deprecated warning)
        sprintf(&data_hex[2*i], "%02X", sha2sum[i]);
    }
}*/

/******************************************************************************
 ** Name:    SHA2_HMAC_SHA256
 *****************************************************************************/
/**
 ** @brief   Generate SREC file HMAC SHA-256
 **
 ** @param data_hex Array to save SHA256 HMAC
 **
 ** @return  none
 *****************************************************************************/
/*void SHA2_SHA256_HMAC(char_t* data_hex)
{
    unsigned char sha2sum[32];
    sha2_context ctx;

    uint8_t     fx_res, i;
    uint32_t    nBytes = 0, h;
    uint64_t    file_size = 0;
    uint32_t    nFrame = 0;
    uchar_t     read_file_data[255]= {0};

    fx_res = (uint8_t) fx_directory_default_set(&sd_fx_media, NULL);
    fx_res = (uint8_t) fx_file_open(&sd_fx_media, &secure_file, UPGRADE_FILENAME, FX_OPEN_FOR_READ);
    if (fx_res == FX_SUCCESS)
    {
        memcpy(&file_size, &secure_file.fx_file_current_file_size, sizeof(uint64_t));
        // Check file length
        nFrame = (uint32_t) ((secure_file.fx_file_current_file_size) / 250 );
        if ((secure_file.fx_file_current_file_size) % 250 )
        {
            nFrame++;
        }
        sha2_hmac_starts(&ctx, 0, 0, 0);
        // Calculate file CRC
        for(h=0; h < nFrame; h++)
        {
            fx_res = (uint8_t) fx_file_read(&secure_file, read_file_data, 250, &nBytes);
            sha2_hmac_update(&ctx, read_file_data, nBytes);
        }
        sha2_hmac_finish(&ctx, sha2sum);

    }
    fx_res = (uint8_t) fx_file_close(&secure_file);
    fx_res = (uint8_t) fx_media_flush (&sd_fx_media);

    for (i = 0 ; i != 32 ; i++)
    {
        //lint -e586 Admissible use of sprintf (deprecated warning)
        sprintf(&data_hex[2*i], "%02X", sha2sum[i]);
    }
}*/

/******************************************************************************
 ** Name:    AES_Algo_Verify
 *****************************************************************************/
/**
 ** @brief   Function to verify signature
 **
 ** @return  ATCA_STATUS
 *****************************************************************************/
/*bool_t AES_Algo_Verify(void)
{
    uint8_t  fx_res;
    UNUSED(fx_res);
    //uchar_t public_key_data[64] = {0};
    char_t msg_hash[64] = {0};
    uint8_t signature[64] = {0};

    // Generate hash from receive firmware
    SHA2_SHA256_HMAC(msg_hash);

    // copy signature from server
    memcpy(signature, wifi_header.password, 64);

    if(memcmp(signature, msg_hash, 64) != 0)
    {
        fx_res = (uint8_t) fx_file_delete(&sd_fx_media, UPGRADE_FILENAME);
        fx_res = (uint8_t) fx_media_flush (&sd_fx_media);
        return eERR_COMM_WIFI_RECEIVE_UPGRADE_FILE;
    }
    else return eERR_NONE;

    return eERR_NONE;
}*/
