#ifndef SRC_INCLUDE_DEVICE_H_
#define SRC_INCLUDE_DEVICE_H_

#if (ELECTRICITY_TYPE == KASKAD_1_MT)
#include "kaskad_1_mt.h"
#elif (ELECTRICITY_TYPE == KASKAD_11)
#endif

extern uint8_t  tariff_changed;
extern uint8_t  pv_changed;


extern void measure_meter();

#endif /* SRC_INCLUDE_DEVICE_H_ */
