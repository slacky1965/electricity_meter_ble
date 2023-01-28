#include <stdint.h>
#include "tl_common.h"
#include "stack/ble/ble.h"
#include "drivers/8258/compiler.h"

#include "ble.h"
#include "cfg.h"
#include "ccm.h"

_attribute_data_retention_ bthome_nonce_t bthome_nonce;

void bthome_beacon_init() {
    bthome_nonce.mac[0] = mac_public[5];
    bthome_nonce.mac[1] = mac_public[4];
    bthome_nonce.mac[2] = mac_public[3];
    bthome_nonce.mac[3] = mac_public[2];
    bthome_nonce.mac[4] = mac_public[1];
    bthome_nonce.mac[5] = mac_public[0];
    bthome_nonce.uuid16 = ADV_BTHOME_UUID16;
    bthome_nonce.device_info = device_info_encrypt | device_info_version;
    bthome_nonce.counter = 0;
    adv_crypt_tariff_data.flg_size  = 0x02;              /* size  */
    adv_crypt_tariff_data.flg_type  = GAP_ADTYPE_FLAGS;  /* 0x01  */
    adv_crypt_tariff_data.flg       = 0x06;              /* flags */

    adv_crypt_tariff_data.head.size = (sizeof(adv_head_uuid16_t)-1 + sizeof(tariff_t) + 8) & 0xFF;
    adv_crypt_tariff_data.head.type = GAP_ADTYPE_SERVICE_DATA_UUID_16BIT;
    adv_crypt_tariff_data.head.UUID = ADV_BTHOME_UUID16;
    adv_crypt_tariff_data.head.device_info = device_info_encrypt | device_info_version;

    adv_crypt_pv_data.flg_size  = 0x02;              /* size  */
    adv_crypt_pv_data.flg_type  = GAP_ADTYPE_FLAGS;  /* 0x01  */
    adv_crypt_pv_data.flg       = 0x06;              /* flags */

    adv_crypt_pv_data.head.size = (sizeof(adv_head_uuid16_t)-1 + sizeof(power_voltage_t) + 8) & 0xFF;
    adv_crypt_pv_data.head.type = GAP_ADTYPE_SERVICE_DATA_UUID_16BIT;
    adv_crypt_pv_data.head.UUID = ADV_BTHOME_UUID16;
    adv_crypt_pv_data.head.device_info = device_info_encrypt | device_info_version;
}


_attribute_ram_code_ __attribute__((optimize("-Os")))
void bthome_encrypt_tariff_data_beacon() {

    bthome_nonce.counter++;
    adv_crypt_tariff_data.counter = bthome_nonce.counter;

    aes_ccm_encrypt_and_tag((const unsigned char *)&config.bindkey,
                       (uint8_t*)&bthome_nonce, sizeof(bthome_nonce_t),
                       0, 0,
                       (uint8_t*)&adv_tariff_data.tariff, sizeof(tariff_t),
                       adv_crypt_tariff_data.ciphertext,
                       adv_crypt_tariff_data.mic, 4);
}

_attribute_ram_code_ __attribute__((optimize("-Os")))
void bthome_encrypt_pv_data_beacon() {

    bthome_nonce.counter++;
    adv_crypt_pv_data.counter = bthome_nonce.counter;

    aes_ccm_encrypt_and_tag((const unsigned char *)&config.bindkey,
                       (uint8_t*)&bthome_nonce, sizeof(bthome_nonce_t),
                       0, 0,
                       (uint8_t*)&adv_pv_data.pv, sizeof(power_voltage_t),
                       adv_crypt_pv_data.ciphertext,
                       adv_crypt_pv_data.mic, 4);
}
