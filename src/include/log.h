#ifndef SRC_INCLUDE_LOG_H_
#define SRC_INCLUDE_LOG_H_

#define LOG_ID        0x78
#define LOG_STR_SIZE  13              /* all 16 bytes. 1 byte id, 1 byte pid, 1 byte debug_enable. 16-3=13 bytes */
#define LOG_BUFF_SIZE 1024            /* size ring buffer  */
#define LOG_BUFF_MASK LOG_BUFF_SIZE-1 /* mask ring buffer  */

typedef struct __attribute__((packed)) _log_notify_t {
    uint8_t  id;                      /* LOG_ID            */
    uint8_t  pid;                     /* 0 .. 255          */
    uint8_t  debug_enabled;           /* 1 - debug enabled */
    uint8_t  str[LOG_STR_SIZE];       /* log string        */
} log_notify_t;

extern uint8_t log_pos;
extern log_notify_t log_notify;
extern uint8_t send_log_enable;

void init_log();
int putchar(int c);
uint8_t log_available();
void log_buff_clear();
void set_log_str();

#endif /* SRC_INCLUDE_LOG_H_ */
