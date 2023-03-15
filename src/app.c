#include <stdint.h>
#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"
#include "drivers/8258/pm.h"
#include "drivers/8258/timer.h"

#include "app.h"
#include "app_att.h"
#include "app_adc.h"
#include "cfg.h"
#include "log.h"
#include "ble.h"
#include "app_uart.h"
#include "device.h"


#if UART_PRINT_DEBUG_ENABLE
//_attribute_data_retention_ static uint32_t deepRetn_count = 0;
#endif /* UART_PRINT_DEBUG_ENABLE */

#define CONN_TIMEOUT        300         /* 5 min */
#define RESET_WL_TIMEOUT    5           /* 5 sec */

_attribute_data_retention_ uint32_t measure_interval;
_attribute_data_retention_ uint32_t battery_interval;
_attribute_data_retention_ uint32_t conn_timeout;

_attribute_data_retention_ uint32_t time_sec_tick;
_attribute_data_retention_ uint32_t time_tick_step = CLOCK_16M_SYS_TIMER_CLK_1S; // adjust time clock (in 1/16 us for 1 sec)
_attribute_data_retention_ uint32_t time_sec = 0;

_attribute_data_retention_ uint8_t tariff1_notify = NOTIFY_MAX;
_attribute_data_retention_ uint8_t tariff2_notify = NOTIFY_MAX;
_attribute_data_retention_ uint8_t tariff3_notify = NOTIFY_MAX;
_attribute_data_retention_ uint8_t power_notify   = NOTIFY_MAX;
_attribute_data_retention_ uint8_t voltage_notify = NOTIFY_MAX;
_attribute_data_retention_ uint8_t ampere_notify  = NOTIFY_MAX;
_attribute_data_retention_ uint8_t sn_notify      = NOTIFY_MAX;
_attribute_data_retention_ uint8_t dr_notify      = NOTIFY_MAX;
_attribute_data_retention_ uint8_t mn_notify      = 0;
_attribute_data_retention_ uint8_t lg_notify      = 0;
_attribute_data_retention_ uint8_t bndk_notify    = 0;

_attribute_data_retention_ uint8_t  reset_wl_begin;
_attribute_data_retention_ uint32_t reset_wl_timeout;

_attribute_ram_code_ void check_reset_wl() {

//    gpio_setup_up_down_resistor(RWL_GPIO, PM_PIN_PULLUP_1M);

    if (!gpio_read(RWL_GPIO)) {
        if (reset_wl_begin) {
            if ((time_sec - reset_wl_timeout) > (RESET_WL_TIMEOUT)) {
#if UART_PRINT_DEBUG_ENABLE
                printf("Reset whitelist\r\n");
#endif /* UART_PRINT_DEBUG_ENABLE */
                bls_smp_eraseAllParingInformation();
                ev_adv_timeout(0,0,0);
                reset_wl_begin = false;
            }
        } else {
            reset_wl_timeout = time_sec;
            reset_wl_begin = true;
        }
    } else {
        reset_wl_begin = false;
    }

//    gpio_setup_up_down_resistor(RWL_GPIO, PM_PIN_UP_DOWN_FLOAT);

}

void user_init_normal(void) {

    init_log();

#if UART_PRINT_DEBUG_ENABLE
    printf("Start user_init_normal()\r\n");
#endif /* UART_PRINT_DEBUG_ENABLE */

    adc_power_on_sar_adc(OFF);
	random_generator_init();  //this is must
    init_config();
    battery_mv = get_battery_mv();
    measure_interval  = 0;
    battery_interval  = 0;
    reset_wl_begin = false;
    flush_buff_uart();
    app_uart_init();
    set_device_type(config.save_data.device_type);
    meter.measure_meter();
    init_ble();
}

_attribute_ram_code_ void user_init_deepRetn(void) {

#if UART_PRINT_DEBUG_ENABLE
        //printf("%u Start user_init_deeptn()\r\n", ++deepRetn_count);
#endif /* UART_PRINT_DEBUG_ENABLE */

	blc_ll_initBasicMCU();   //mandatory
	rf_set_power_level_index (MY_RF_POWER_INDEX);

	blc_ll_recoverDeepRetention();

    app_uart_init();

	irq_enable();
}

_attribute_ram_code_ void blt_pm_proc(void)
{
    if(ota_is_working){
        bls_pm_setSuspendMask (SUSPEND_ADV | SUSPEND_CONN); // SUSPEND_DISABLE
        bls_pm_setManualLatency(0);
    }else{
        bls_pm_setSuspendMask (SUSPEND_ADV | DEEPSLEEP_RETENTION_ADV | SUSPEND_CONN | DEEPSLEEP_RETENTION_CONN);
    }
}

void main_loop (void) {

    blt_sdk_main_loop();

    while (clock_time() -  time_sec_tick > time_tick_step) {
        time_sec_tick += time_tick_step;
        time_sec++; // + 1 sec
    }

    if(!ota_is_working) {

        if ((time_sec - measure_interval) > (config.measurement_period*60)) {

            meter.measure_meter();

            measure_interval = time_sec;
        }

//        if ((time_sec - battery_interval) > CONN_TIMEOUT) {
//            battery_mv = get_battery_mv();
//            if (battery_mv != adv_pva_data.pv.voltage3_3) {
//                if ((battery_mv > adv_pva_data.pv.voltage3_3 && (battery_mv - adv_pva_data.pv.voltage3_3) > 50) ||
//                    (battery_mv < adv_pva_data.pv.voltage3_3 && (adv_pva_data.pv.voltage3_3 - battery_mv) > 50)) {
//#if UART_PRINT_DEBUG_ENABLE
//                    printf("New battery mv - %u, last battery mv - %u\r\n", battery_mv, adv_pva_data.pv.voltage3_3);
//#endif /* UART_PRINT_DEBUG_ENABLE */
//                   pva_changed = true;
//                }
//            }
//            battery_interval = time_sec;
//        }

        check_reset_wl();

        if(blc_ll_getCurrentState() & BLS_LINK_STATE_CONN) {
            if(blc_ll_getTxFifoNumber() < 9) {
                if (RxTxValueInCCC) {
                    if (mn_notify) {
                        ble_send_main();
                        mn_notify--;
                    } else if (bndk_notify) {
                        ble_send_bindkey();
                        bndk_notify--;
                    } else if (sn_notify) {
                        ble_send_serila_number();
                        sn_notify--;
                    } else if (dr_notify) {
                        ble_send_date_release();
                        dr_notify--;
                    } else {
                        if (send_log_enable ) {
                            if (lg_notify) {
                                ble_send_log();
                                lg_notify--;
                            } else {
                                if (log_notify.debug_enabled && log_available()) {
                                    set_log_str();
                                }
                            }
                        }
                    }
                }

                if (tariff1ValueInCCC) {
                    if (tariff1_notify) {
                        ble_send_tariff1();
                        tariff1_notify--;
                    }
                }

                if (tariff2ValueInCCC) {
                    if (tariff2_notify) {
                        ble_send_tariff2();
                        tariff2_notify--;
                    }
                }
                if (tariff3ValueInCCC) {
                    if (tariff3_notify) {
                        ble_send_tariff3();
                        tariff3_notify--;
                    }
                }
                if (powerValueInCCC) {
                    if (power_notify) {
                        ble_send_power();
                        power_notify--;
                    }
                }
                if (voltageValueInCCC) {
                    if (voltage_notify) {
                        ble_send_voltage();
                        voltage_notify--;
                    }
                }
                if (ampereValueInCCC) {
                    if (ampere_notify) {
                        ble_send_ampere();
                        ampere_notify--;
                    }
                }
            }

            /* connection time 5 min. */
            if ((time_sec - conn_timeout) > (CONN_TIMEOUT)) {
#if UART_PRINT_DEBUG_ENABLE
                printf("Connection timeout %u min %u sec.\r\n", CONN_TIMEOUT/60, CONN_TIMEOUT%60);
#endif /* UART_PRINT_DEBUG_ENABLE */
                ble_connected |= conn_delayed_disconnect;
                conn_timeout = time_sec;
            }
        }

        if (ble_connected & conn_delayed_disconnect) {
            bls_ll_terminateConnection(HCI_ERR_REMOTE_USER_TERM_CONN);
            return;
        }

    }
    blt_pm_proc();
}


