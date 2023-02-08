#ifndef SRC_INCLUDE_DEVICE_H_
#define SRC_INCLUDE_DEVICE_H_

typedef enum _pkt_error_t {
    PKT_OK  = 0,
    PKT_ERR_NO_PKT,
    PKT_ERR_TIMEOUT,
    PKT_ERR_UNKNOWN_FORMAT,
    PKT_ERR_DIFFERENT_COMMAND,
    PKT_ERR_INCOMPLETE,
    PKT_ERR_UNSTUFFING,
    PKT_ERR_ADDRESS,
    PKT_ERR_RESPONSE,
    PKT_ERR_CRC,
    PKT_ERR_UART
} pkt_error_t;

#if (ELECTRICITY_TYPE == KASKAD_1_MT)
#include "kaskad_1_mt.h"
#elif (ELECTRICITY_TYPE == KASKAD_11)
#endif

extern uint8_t  tariff_changed;
extern uint8_t  pv_changed;

extern void measure_meter();

#endif /* SRC_INCLUDE_DEVICE_H_ */
