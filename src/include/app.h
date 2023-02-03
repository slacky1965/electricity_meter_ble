#ifndef SRC_INCLUDE_APP_H_
#define SRC_INCLUDE_APP_H_

#define NOTIFY_MAX 2

extern uint32_t conn_timeout;
extern uint32_t time_sec;
extern uint8_t tariff1_notify;
extern uint8_t tariff2_notify;
extern uint8_t tariff3_notify;
extern uint8_t power_notify;
extern uint8_t voltage_notify;
extern uint8_t mn_notify;
extern uint8_t lg_notify;
extern uint8_t bndk_notify;
extern uint8_t sn_notify;
extern uint8_t dr_notify;

void user_init_normal(void);
void user_init_deepRetn(void);
void main_loop (void);

#endif /* SRC_INCLUDE_APP_H_ */
