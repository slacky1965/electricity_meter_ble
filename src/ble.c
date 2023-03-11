#include <stdint.h>
#include "common/string.h"
#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"
#include "drivers/8258/pm.h"

#include "log.h"
#include "ble.h"
#include "app.h"
#include "app_att.h"
#include "app_adc.h"
#include "cfg.h"
#include "cmd_parser.h"

_attribute_data_retention_ uint8_t              ble_name[BLE_NAME_SIZE];
_attribute_data_retention_ adv_tariff_t         adv_tariff_data;
_attribute_data_retention_ adv_power_voltage_amps_t  adv_pva_data;
_attribute_data_retention_ adv_crypt_tariff_t   adv_crypt_tariff_data;
_attribute_data_retention_ adv_crypt_power_voltage_amps_t  adv_crypt_pva_data;
_attribute_data_retention_ main_notify_t        main_notify;
_attribute_data_retention_ bindkey_notify_t     bindkey_notify;
_attribute_data_retention_ serial_number_notify_t serial_number_notify;
_attribute_data_retention_ date_release_notify_t date_release_notify;
_attribute_data_retention_ uint8_t              mac_public[6], mac_random_static[6];
_attribute_data_retention_ uint16_t             ble_connected = conn_disconnect;
_attribute_data_retention_ uint8_t              ota_is_working = 0;
_attribute_data_retention_ uint8_t              first_start;
_attribute_data_retention_ uint8_t              adv_counter;

_attribute_data_retention_ uint8_t              blt_rxfifo_b[64 * 8] = {0};
_attribute_data_retention_ my_fifo_t            blt_rxfifo = { 64, 8, 0, 0, blt_rxfifo_b,};
_attribute_data_retention_ uint8_t              blt_txfifo_b[40 * 16] = {0};
_attribute_data_retention_ my_fifo_t            blt_txfifo = { 40, 16, 0, 0, blt_txfifo_b,};


#define APP_ADV_SETS_NUMBER                 1
#define APP_MAX_LENGTH_ADV_DATA             1024
#define APP_MAX_LENGTH_SCAN_RESPONSE_DATA   31

#if 0
_attribute_data_retention_  uint8_t  app_adv_set_param[ADV_SET_PARAM_LENGTH * APP_ADV_SETS_NUMBER];
_attribute_data_retention_  uint8_t  app_primary_adv_pkt[MAX_LENGTH_PRIMARY_ADV_PKT * APP_ADV_SETS_NUMBER];
_attribute_data_retention_  uint8_t  app_secondary_adv_pkt[MAX_LENGTH_SECOND_ADV_PKT * APP_ADV_SETS_NUMBER];
_attribute_data_retention_  uint8_t  app_advData[APP_MAX_LENGTH_ADV_DATA * APP_ADV_SETS_NUMBER];
_attribute_data_retention_  uint8_t  app_scanRspData[APP_MAX_LENGTH_SCAN_RESPONSE_DATA * APP_ADV_SETS_NUMBER];
#endif

#if UART_PRINT_DEBUG_ENABLE
void print_mac(uint8_t num, uint8_t *mac) {
    printf("MAC%u from whitelist: ", num);
    for (uint8_t i = 0; i < 6; i++) {
        printf("0x%X ", mac[i]);
    }
    printf("\r\n");

}
#endif /* UART_PRINT_DEBUG_ENABLE */

void from32to24(uint8_t *str, uint32_t val) {
    str[2] = (val >> 16) & 0xFF;
    str[1] = (val >> 8) & 0xFF;
    str[0] = val & 0xFF;
}

void app_enter_ota_mode(void)
{
    ota_is_working = true;
    bls_pm_setManualLatency(0);
    bls_ota_setTimeout(40 * 1000000); // set OTA timeout  40 seconds
#if UART_PRINT_DEBUG_ENABLE
    printf("Start OTA update\r\n");
#endif /* UART_PRINT_DEBUG_ENABLE */
}

#if UART_PRINT_DEBUG_ENABLE
void app_debug_ota_result(int result) {

    printf("OTA update return code: 0x%X - ", result);

        switch(result) {

            case OTA_SUCCESS:
                printf("OTA_SUCCESS\r\n");
                break;
            case OTA_DATA_PACKET_SEQ_ERR:
                printf("OTA_DATA_PACKET_SEQ_ERR\r\n");
                break;
            case OTA_PACKET_INVALID:
                printf("OTA_PACKET_INVALID\r\n");
                break;
            case OTA_DATA_CRC_ERR:
                printf("OTA_DATA_CRC_ERR\r\n");
                break;
            case OTA_WRITE_FLASH_ERR :
                printf("OTA_WRITE_FLASH_ERR\r\n");
                break;
            case OTA_DATA_UNCOMPLETE:
                printf("OTA_DATA_UNCOMPLETE\r\n");
                break;
            case OTA_FLOW_ERR:
                printf("OTA_FLOW_ERR\r\n");
                break;
            case OTA_FW_CHECK_ERR:
                printf("OTA_FLOW_ERR\r\n");
                break;
            case OTA_VERSION_COMPARE_ERR:
                printf("OTA_VERSION_COMPARE_ERR\r\n");
                break;
            case OTA_PDU_LEN_ERR:
                printf("OTA_PDU_LEN_ERR\r\n");
                break;
            case OTA_FIRMWARE_MARK_ERR:
                printf("OTA_FIRMWARE_MARK_ERR\r\n");
                break;
            case OTA_FW_SIZE_ERR:
                printf("OTA_FW_SIZE_ERR\r\n");
                break;
            case OTA_DATA_PACKET_TIMEOUT:
                printf("OTA_DATA_PACKET_TIMEOUT\r\n");
                break;
            case OTA_TIMEOUT:
                printf("OTA_TIMEOUT\r\n");
                break;
            case OTA_FAIL_DUE_TO_CONNECTION_TERMIANTE:
                printf("OTA_FAIL_DUE_TO_CONNECTION_TERMIANTE\r\n");
                break;
            case OTA_LOGIC_ERR:
                printf("OTA_LOGIC_ERR\r\n");
                break;
            default:
                printf("Unknown\r\n");
                break;
        }

}
#endif /* UART_PRINT_DEBUG_ENABLE */


int RxTxWrite(void * p)
{
    cmd_parser(p);
    return 0;
}

_attribute_ram_code_ static void suspend_enter_cb(uint8_t e, uint8_t *p, int n) {
    (void) e; (void) p; (void) n;
    bls_pm_setWakeupSource(/*PM_WAKEUP_PAD |*/ PM_WAKEUP_TIMER);  // gpio pad wakeup suspend/deepsleep
}

_attribute_ram_code_ void suspend_exit_cb (uint8_t e, uint8_t *p, int n)
{
    rf_set_power_level_index (RF_POWER_P3p01dBm);
}

void ble_connect_cb(uint8_t e, uint8_t *p, int n) {

    log_buff_clear();

#if UART_PRINT_DEBUG_ENABLE
    printf("Connect\r\n");
#endif /* UART_PRINT_DEBUG_ENABLE */

    ble_connected = conn_connect;
    bls_l2cap_requestConnParamUpdate (CONN_INTERVAL_10MS, CONN_INTERVAL_10MS, 249, 800);  // 2.5 S
    conn_timeout = time_sec;

    tariff1_notify = 0;
    tariff2_notify = 0;
    tariff3_notify = 0;
    power_notify   = 0;
    voltage_notify = 0;
    ampere_notify = 0;
}

void ble_disconnect_cb(uint8_t e,uint8_t *p, int n) {

#if UART_PRINT_DEBUG_ENABLE
    printf("Disconnect\r\n");
#endif /* UART_PRINT_DEBUG_ENABLE */

    if (first_start) {
        uint8_t bond_number = blc_smp_param_getCurrentBondingDeviceNumber();  //get bonded device number
        /* get latest device info */
        if (bond_number) {
#if UART_PRINT_DEBUG_ENABLE
            printf("Bonded\r\n");
#endif /* UART_PRINT_DEBUG_ENABLE */
            first_start = 0;
            ev_adv_timeout(0, 0, 0);
        }
    }

    ota_is_working = false;

    log_buff_clear();
    send_log_enable = false;

    if (ble_connected & conn_delayed_reset) {
        bls_ll_terminateConnection(HCI_ERR_REMOTE_USER_TERM_CONN);
        sleep_ms(2000);
#if UART_PRINT_DEBUG_ENABLE
        printf("Reboot module\r\n");
#endif /* UART_PRINT_DEBUG_ENABLE */
        start_reboot();
    }

    ble_connected = conn_disconnect;

}

void ev_adv_timeout(uint8_t e, uint8_t *p, int n) {
    smp_param_save_t  bondInfo;
    uint8_t bond_number = blc_smp_param_getCurrentBondingDeviceNumber();  //get bonded device number
    /* get latest device info */
    if (bond_number) {
        bls_smp_param_loadByIndex( bond_number - 1, &bondInfo);  //get the latest bonding device (index: bond_number-1 )
    }

    ll_whiteList_reset();     //clear whitelist
    ll_resolvingList_reset(); //clear resolving list

    /* use whitelist to filter master device */
    if (bond_number) {
        //if master device use RPA(resolvable private address), must add irk to resolving list
        if (IS_RESOLVABLE_PRIVATE_ADDR(bondInfo.peer_addr_type, bondInfo.peer_addr)){
            /* resolvable private address, should add peer irk to resolving list */
            ll_resolvingList_add(bondInfo.peer_id_adrType, bondInfo.peer_id_addr, bondInfo.peer_irk, NULL);  //no local IRK
            ll_resolvingList_setAddrResolutionEnable(ON);
        } else {
            //if not resolvable random address, add peer address to whitelist
            ll_whiteList_add(bondInfo.peer_addr_type, bondInfo.peer_addr);
        }


#if 1
        bls_ll_setAdvParam( ADV_INTERVAL_MIN, ADV_INTERVAL_MAX,
                            ADV_TYPE_CONNECTABLE_UNDIRECTED, OWN_ADDRESS_PUBLIC,
                            0,  NULL, BLT_ENABLE_ADV_ALL, ADV_FP_ALLOW_SCAN_ANY_ALLOW_CONN_WL);
#else
        blc_ll_setExtAdvParam(ADV_HANDLE0, ADV_EVT_PROP_EXTENDED_CONNECTABLE_UNDIRECTED,
                              ADV_INTERVAL_MIN, ADV_INTERVAL_MAX,
                              BLT_ENABLE_ADV_ALL, OWN_ADDRESS_PUBLIC, BLE_ADDR_PUBLIC, NULL,
                              ADV_FP_ALLOW_SCAN_ANY_ALLOW_CONN_WL, MY_RF_POWER_INDEX, BLE_PHY_1M, 0,
                              BLE_PHY_1M, ADV_SID_0, 0);
#endif

#if UART_PRINT_DEBUG_ENABLE
        printf("Bound, start with whitelist!\r\n");
#endif /* UART_PRINT_DEBUG_ENABLE */
    } else {

#if 1
        bls_ll_setAdvParam( ADV_INTERVAL_250MS, ADV_INTERVAL_300MS,
                            ADV_TYPE_CONNECTABLE_UNDIRECTED, OWN_ADDRESS_PUBLIC,
                            0,  NULL, BLT_ENABLE_ADV_ALL, ADV_FP_NONE);

#else

        blc_ll_setExtAdvParam(ADV_HANDLE0, ADV_EVT_PROP_EXTENDED_CONNECTABLE_UNDIRECTED,
                              ADV_INTERVAL_250MS, ADV_INTERVAL_300MS,
                              BLT_ENABLE_ADV_ALL, OWN_ADDRESS_PUBLIC, BLE_ADDR_PUBLIC, NULL,
                              ADV_FP_NONE, MY_RF_POWER_INDEX, BLE_PHY_1M, 0,
                              BLE_PHY_1M, ADV_SID_0, 0);
#endif

        first_start = 1;
#if UART_PRINT_DEBUG_ENABLE
        printf("Not bound, first start!\r\n");
#endif /* UART_PRINT_DEBUG_ENABLE */

    }

#if 1
    bls_ll_setScanRspData((uint8_t*)ble_name, ble_name[0]+1);
    bls_ll_setAdvEnable(BLC_ADV_ENABLE);  //ADV enable
#else
    blc_ll_setExtScanRspData(ADV_HANDLE0, DATA_OPER_COMPLETE, DATA_FRAGM_ALLOWED, ble_name[0]+1, (uint8_t*)ble_name);
    blc_ll_setExtAdvEnable_1(BLC_ADV_ENABLE, 1, ADV_HANDLE0, 0 , 0);
#endif
}

_attribute_ram_code_ int app_advertise_prepare_handler(rf_packet_adv_t * p)  {
    (void) p;
    if (adv_counter & 1) {
        if (pva_changed) {
            pva_changed = false;
            adv_pva_data.pva.pid++;

            uint32_t power;
            if (config.save_data.divisor.power_sign) {
                power = meter.power * get_divisor(config.save_data.divisor.power_divisor);
            } else {
                power = meter.power / get_divisor(config.save_data.divisor.power_divisor);
            }
            from32to24(adv_pva_data.pva.power, power);

            uint16_t amps_volts;
            if (config.save_data.divisor.voltage_sign) {
                amps_volts = meter.voltage * get_divisor(config.save_data.divisor.voltage_divisor);
            } else {
                amps_volts = meter.voltage / get_divisor(config.save_data.divisor.voltage_divisor);
            }
            adv_pva_data.pva.voltage220 = amps_volts;

            adv_pva_data.pva.battery_level = meter.battery_level;

            if (config.save_data.divisor.current_sign) {
                amps_volts = meter.amps * get_divisor(config.save_data.divisor.current_divisor);
            } else {
                amps_volts = meter.amps / get_divisor(config.save_data.divisor.current_divisor);
            }
            adv_pva_data.pva.amps = amps_volts;

//            adv_pva_data.pva.voltage3_3 = battery_mv;

            if (config.save_data.encrypted) {
                bthome_encrypt_pva_data_beacon();
            }
        }
        if (config.save_data.encrypted) {
            bls_ll_setAdvData((uint8_t*)&adv_crypt_pva_data, sizeof(adv_crypt_power_voltage_amps_t));
        } else {
            bls_ll_setAdvData((uint8_t*)&adv_pva_data, sizeof(adv_power_voltage_amps_t));
        }
    } else {
        if (tariff_changed) {
            tariff_changed = false;
            adv_tariff_data.tariff.pid++;

            uint32_t tariff;
            if (config.save_data.divisor.tariffs_sign) {
                tariff = meter.tariff_1 * get_divisor(config.save_data.divisor.tariffs_divisor);
                from32to24(adv_tariff_data.tariff.tariff_1, tariff);
                tariff = meter.tariff_2 * get_divisor(config.save_data.divisor.tariffs_divisor);
                from32to24(adv_tariff_data.tariff.tariff_2, tariff);
                tariff = meter.tariff_3 * get_divisor(config.save_data.divisor.tariffs_divisor);
                from32to24(adv_tariff_data.tariff.tariff_3, tariff);
            } else {
                tariff = meter.tariff_1 / get_divisor(config.save_data.divisor.tariffs_divisor);
                from32to24(adv_tariff_data.tariff.tariff_1, tariff);
                tariff = meter.tariff_2 / get_divisor(config.save_data.divisor.tariffs_divisor);
                from32to24(adv_tariff_data.tariff.tariff_2, tariff);
                tariff = meter.tariff_3 / get_divisor(config.save_data.divisor.tariffs_divisor);
                from32to24(adv_tariff_data.tariff.tariff_3, tariff);
            }

            if (config.save_data.encrypted) {
                bthome_encrypt_tariff_data_beacon();
            }
        }
        if (config.save_data.encrypted) {
            bls_ll_setAdvData((uint8_t*)&adv_crypt_tariff_data, sizeof(adv_crypt_tariff_t));
        } else {
            bls_ll_setAdvData((uint8_t*)&adv_tariff_data, sizeof(adv_tariff_t));
        }
    }
    adv_counter++;
    return 1;
}


extern attribute_t my_Attributes[ATT_END_H];
const char* hex_ascii = {"0123456789ABCDEF"};

void get_ble_name() {
    uint8_t *blename = config.ble_name;
    uint8_t pos = sizeof(DEV_NAME_STR)-1;

    memcpy(ble_name+2, DEV_NAME_STR, pos);
    pos +=2;
    ble_name[pos++] = '_';
    ble_name[pos++] = hex_ascii[mac_public[2]>>4];
    ble_name[pos++] = hex_ascii[mac_public[2] &0x0f];
    ble_name[pos++] = hex_ascii[mac_public[1]>>4];
    ble_name[pos++] = hex_ascii[mac_public[1] &0x0f];
    ble_name[pos++] = hex_ascii[mac_public[0]>>4];
    ble_name[pos++] = hex_ascii[mac_public[0] &0x0f];
    ble_name[0]     = BLE_NAME_SIZE-1;
    ble_name[1]     = GAP_ADTYPE_LOCAL_NAME_COMPLETE;

    if (*blename == 0) {
        memcpy(blename, ble_name, ble_name[0]+1);
    } else {
        if (memcmp(ble_name, blename, BLE_NAME_SIZE)) {
            memcpy(blename, ble_name, BLE_NAME_SIZE);
        }
    }
    my_Attributes[GenericAccess_DeviceName_DP_H].attrLen = ble_name[0] - 1;
}

__attribute__((optimize("-Os"))) void init_ble(void) {

    blc_initMacAddress(FLASH_SECTOR_MAC, mac_public, mac_random_static);
    /// if bls_ll_setAdvParam( OWN_ADDRESS_RANDOM ) ->  blc_ll_setRandomAddr(mac_random_static);
    get_ble_name();

    adv_counter = 0;

    adv_tariff_data.flg_size  = 0x02;              /* size  */
    adv_tariff_data.flg_type  = GAP_ADTYPE_FLAGS;  /* 0x01  */
    adv_tariff_data.flg       = 0x06;              /* flags */

    adv_tariff_data.head.size = (sizeof(adv_head_uuid16_t)-1 + sizeof(tariff_t)) & 0xFF;
    adv_tariff_data.head.type = GAP_ADTYPE_SERVICE_DATA_UUID_16BIT;
    adv_tariff_data.head.UUID = ADV_BTHOME_UUID16;
    adv_tariff_data.head.device_info = device_info_encrypt_none | device_info_version;

    adv_tariff_data.tariff.tariff1_id = BTHomeID_packet_id;
    adv_tariff_data.tariff.pid = 0;

    adv_tariff_data.tariff.tariff1_id = BTHomeID_energy;
    adv_tariff_data.tariff.tariff2_id = BTHomeID_energy;
    adv_tariff_data.tariff.tariff3_id = BTHomeID_energy;

    uint32_t tariff;
    if (config.save_data.divisor.tariffs_sign) {
        tariff = meter.tariff_1 * get_divisor(config.save_data.divisor.tariffs_divisor);
        from32to24(adv_tariff_data.tariff.tariff_1, tariff);
        tariff = meter.tariff_2 * get_divisor(config.save_data.divisor.tariffs_divisor);
        from32to24(adv_tariff_data.tariff.tariff_2, tariff);
        tariff = meter.tariff_3 * get_divisor(config.save_data.divisor.tariffs_divisor);
        from32to24(adv_tariff_data.tariff.tariff_3, tariff);
    } else {
        tariff = meter.tariff_1 / get_divisor(config.save_data.divisor.tariffs_divisor);
        from32to24(adv_tariff_data.tariff.tariff_1, tariff);
        tariff = meter.tariff_2 / get_divisor(config.save_data.divisor.tariffs_divisor);
        from32to24(adv_tariff_data.tariff.tariff_2, tariff);
        tariff = meter.tariff_3 / get_divisor(config.save_data.divisor.tariffs_divisor);
        from32to24(adv_tariff_data.tariff.tariff_3, tariff);
    }


    adv_pva_data.flg_size  = 0x02;              /* size  */
    adv_pva_data.flg_type  = GAP_ADTYPE_FLAGS;  /* 0x01  */
    adv_pva_data.flg       = 0x06;              /* flags */

    adv_pva_data.head.size = (sizeof(adv_head_uuid16_t)-1 + sizeof(power_voltage_amps_t)) & 0xFF;
    adv_pva_data.head.type = GAP_ADTYPE_SERVICE_DATA_UUID_16BIT;
    adv_pva_data.head.UUID = ADV_BTHOME_UUID16;
    adv_pva_data.head.device_info = device_info_encrypt_none | device_info_version;

    adv_pva_data.pva.pkt_id = BTHomeID_packet_id;
    adv_pva_data.pva.pid = 0;

    adv_pva_data.pva.power_id = BTHomeID_power;
    uint32_t power;
    if (config.save_data.divisor.power_sign) {
        power = meter.power * get_divisor(config.save_data.divisor.power_divisor);
    } else {
        power = meter.power / get_divisor(config.save_data.divisor.power_divisor);
    }
    from32to24(adv_pva_data.pva.power, power);

    adv_pva_data.pva.voltage220_id = BTHomeID_voltage;
    uint16_t amps_volts;
    if (config.save_data.divisor.voltage_sign) {
        amps_volts = meter.voltage * get_divisor(config.save_data.divisor.voltage_divisor);
    } else {
        amps_volts = meter.voltage / get_divisor(config.save_data.divisor.voltage_divisor);
    }
    adv_pva_data.pva.voltage220 = amps_volts;
    printf("volts :%u, volts_div: %u\r\n", meter.voltage, amps_volts);

    adv_pva_data.pva.amps_id = BTHomeID_current;
    if (config.save_data.divisor.current_sign) {
        amps_volts = meter.amps * get_divisor(config.save_data.divisor.current_divisor);
    } else {
        amps_volts = meter.amps / get_divisor(config.save_data.divisor.current_divisor);
    }
    adv_pva_data.pva.amps = amps_volts;

    adv_pva_data.pva.battery_id = BTHomeID_battery;
    adv_pva_data.pva.battery_level = meter.battery_level;

//    adv_pva_data.pva.voltage3_3_id = BTHomeID_voltage_001;
//    adv_pva_data.pva.voltage3_3 = battery_mv;

    ///////////////////// Controller Initialization /////////////////////
    blc_ll_initBasicMCU();                      //mandatory
    blc_ll_initStandby_module(mac_public);      //mandatory
#if 1
    blc_ll_initAdvertising_module(mac_public);  //adv module:        mandatory for BLE slave,
#else
    blc_ll_initExtendedAdvertising_module(app_adv_set_param, app_primary_adv_pkt, APP_ADV_SETS_NUMBER);
    blc_ll_initExtSecondaryAdvPacketBuffer(app_secondary_adv_pkt, MAX_LENGTH_SECOND_ADV_PKT);
    blc_ll_initExtAdvDataBuffer(app_advData, APP_MAX_LENGTH_ADV_DATA);
    blc_ll_initExtScanRspDataBuffer(app_scanRspData, APP_MAX_LENGTH_SCAN_RESPONSE_DATA);
#endif

    blc_ll_initConnection_module();             //connection module  mandatory for BLE slave/master
    blc_ll_initSlaveRole_module();              //slave module:      mandatory for BLE slave,

    ///////////////////// Host Initialization /////////////////////
    blc_gap_peripheral_init();    //gap initialization
    my_att_init (); //gatt initialization
    blc_l2cap_register_handler (blc_l2cap_packet_receive);      //l2cap initialization
    blc_smp_peripheral_init();


    ///////////////////// USER application initialization ///////////////////
    rf_set_power_level_index(MY_RF_POWER_INDEX);
    bls_app_registerEventCallback(BLT_EV_FLAG_SUSPEND_ENTER, &suspend_enter_cb);
    bls_app_registerEventCallback(BLT_EV_FLAG_SUSPEND_EXIT, &suspend_exit_cb);
    bls_app_registerEventCallback(BLT_EV_FLAG_CONNECT, &ble_connect_cb);
    bls_app_registerEventCallback(BLT_EV_FLAG_TERMINATE, &ble_disconnect_cb);

    ///////////////////// Power Management initialization///////////////////
    blc_ll_initPowerManagement_module();
    bls_pm_setSuspendMask(SUSPEND_DISABLE);
    blc_pm_setDeepsleepRetentionThreshold(40, 18);
    blc_pm_setDeepsleepRetentionEarlyWakeupTiming(200); // 240
    blc_pm_setDeepsleepRetentionType(DEEPSLEEP_MODE_RET_SRAM_LOW32K);

    bls_ota_clearNewFwDataArea(); //must
    bls_ota_registerStartCmdCb(app_enter_ota_mode);
#if UART_PRINT_DEBUG_ENABLE
    bls_ota_registerResultIndicateCb(app_debug_ota_result);  //debug
#endif /* UART_PRINT_DEBUG_ENABLE */

    bls_set_advertise_prepare(app_advertise_prepare_handler); // todo: not work if EXTENDED_ADVERTISING

    bls_app_registerEventCallback (BLT_EV_FLAG_ADV_DURATION_TIMEOUT, &ev_adv_timeout);

    if (config.save_data.encrypted) {
        bthome_beacon_init();
    }

    pva_changed = tariff_changed = true;

    ev_adv_timeout(0,0,0);
}

void set_adv_data(uint8_t *adv_data, uint8_t data_size) {
    bls_ll_setAdvData(adv_data, data_size);
}

void ble_send_tariff1() {
    bls_att_pushNotifyData(TARIFF1_LEVEL_INPUT_DP_H, (uint8_t *)&meter.tariff_1, sizeof(meter.tariff_1));
}

void ble_send_tariff2() {
    bls_att_pushNotifyData(TARIFF2_LEVEL_INPUT_DP_H, (uint8_t *)&meter.tariff_2, sizeof(meter.tariff_2));
}

void ble_send_tariff3() {
    bls_att_pushNotifyData(TARIFF3_LEVEL_INPUT_DP_H, (uint8_t *)&meter.tariff_3, sizeof(meter.tariff_3));
}

void ble_send_power() {
    bls_att_pushNotifyData(POWER_LEVEL_INPUT_DP_H, (uint8_t *)&meter.power, sizeof(meter.power));
}

void ble_send_voltage() {
    bls_att_pushNotifyData(VOLTAGE_LEVEL_INPUT_DP_H, (uint8_t *)&meter.voltage, sizeof(meter.voltage));
}

void ble_send_ampere() {
    bls_att_pushNotifyData(AMPERE_LEVEL_INPUT_DP_H, (uint8_t *)&meter.amps, sizeof(meter.amps));
}

void ble_send_main() {
    bls_att_pushNotifyData(RxTx_CMD_OUT_DP_H, (uint8_t *)&main_notify, sizeof(main_notify_t));
}

void ble_send_log() {
    bls_att_pushNotifyData(RxTx_CMD_OUT_DP_H, (uint8_t *)&log_notify, sizeof(log_notify_t));
}

void ble_send_bindkey() {
    bls_att_pushNotifyData(RxTx_CMD_OUT_DP_H, (uint8_t *)&bindkey_notify, sizeof(bindkey_notify_t));
}

void ble_send_serila_number() {
    bls_att_pushNotifyData(RxTx_CMD_OUT_DP_H, (uint8_t *)&serial_number_notify, sizeof(serial_number_notify_t));
}

void ble_send_date_release() {
    bls_att_pushNotifyData(RxTx_CMD_OUT_DP_H, (uint8_t *)&date_release_notify, sizeof(date_release_notify_t));
}
