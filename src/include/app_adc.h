#ifndef SRC_INCLUDE_APP_ADC_H_
#define SRC_INCLUDE_APP_ADC_H_

#define get_battery_mv() get_adc_mv(SHL_ADC_VBAT)

u16 get_adc_mv(ADC_InputPchTypeDef p_ain);

extern uint16_t battery_mv;


#endif /* SRC_INCLUDE_APP_ADC_H_ */
