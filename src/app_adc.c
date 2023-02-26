#include <stdint.h>
#include "tl_common.h"
#include "drivers.h"

#define ADC_BUF_COUNT   8

_attribute_data_retention_ uint16_t battery_mv;         /* 2200 ... 3100 mv                 */

ADC_InputPchTypeDef adc_hw_initialized = 0;

_attribute_ram_code_ static void app_adc_init(ADC_InputPchTypeDef p_ain) {

        adc_enable_clk_24m_to_sar_adc(1);
        adc_set_sample_clk(5); //adc sample clk= 24M/(1+5)=4M
        adc_set_left_gain_bias(GAIN_STAGE_BIAS_PER100);
        adc_set_right_gain_bias(GAIN_STAGE_BIAS_PER100);
        adc_set_chn_enable_and_max_state_cnt(ADC_MISC_CHN, 2);
        adc_set_state_length(240, 0, 10);
        adc_set_ain_channel_differential_mode(ADC_MISC_CHN, p_ain, GND);
        adc_set_resolution(ADC_MISC_CHN, RES14);  //set resolution
        adc_set_ref_voltage(ADC_MISC_CHN, ADC_VREF_1P2V);
        adc_set_tsample_cycle(ADC_MISC_CHN, SAMPLING_CYCLES_6);
        adc_set_ain_pre_scaler(ADC_PRESCALER_1F8);
}

_attribute_ram_code_ uint16_t get_adc_mv(ADC_InputPchTypeDef p_ain) {
    volatile unsigned int adc_data_buf[ADC_BUF_COUNT];
    uint16_t temp;

    int i,j;
    uint16_t adc_sample[ADC_BUF_COUNT] = {0};

    if (adc_hw_initialized != p_ain) {
        adc_power_on_sar_adc(0);
        adc_hw_initialized = p_ain;
        app_adc_init(p_ain);
    }

    adc_power_on_sar_adc(1);
    adc_reset_adc_module();
    uint32_t t0 = clock_time();

    for(i=0;i<ADC_BUF_COUNT;i++) {
        adc_data_buf[i] = 0;
    }

    while(!clock_time_exceed(t0, 25));  //wait at least 2 sample cycle(f = 96K, T = 10.4us)
    adc_config_misc_channel_buf((uint16_t*)adc_data_buf, ADC_BUF_COUNT<<2);
    dfifo_enable_dfifo2();
    for(i=0; i < ADC_BUF_COUNT; i++) {
        while((!adc_data_buf[i])&&(!clock_time_exceed(t0,20)));
        t0 = clock_time();
        if(adc_data_buf[i] & BIT(13)) {
            adc_sample[i] = 0;
        }
        else{
            adc_sample[i] = ((uint16_t)adc_data_buf[i] & 0x1FFF);  //BIT(12..0) is valid adc result
        }
        if(i){
            if(adc_sample[i] < adc_sample[i-1]){
                temp = adc_sample[i];
                adc_sample[i] = adc_sample[i-1];
                for(j=i-1;j>=0 && adc_sample[j] > temp;j--){
                    adc_sample[j+1] = adc_sample[j];
                }
                adc_sample[j+1] = temp;
            }
        }
    }
    dfifo_disable_dfifo2();
    adc_power_on_sar_adc(0);
    unsigned int adc_average = (adc_sample[2] + adc_sample[3] + adc_sample[4] + adc_sample[5])/4;
    return (adc_average * adc_vref_cfg.adc_vref)>>10;
}
