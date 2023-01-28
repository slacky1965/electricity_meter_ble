#include <stdarg.h>
#include "stdint.h"

#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"

#include "app.h"
#include "log.h"
#include "ble.h"

_attribute_data_retention_ log_notify_t log_notify;
_attribute_data_retention_ uint8_t send_log_enable;
_attribute_data_retention_ uint8_t log_buff[LOG_BUFF_SIZE];
_attribute_data_retention_ uint16_t log_head, log_tail;

_attribute_ram_code_ uint8_t log_available() {
    if (log_head != log_tail) {
        return true;
    }
    return false;
}

_attribute_ram_code_ void log_buff_clear() {
    log_head = log_tail = 0;
    memset(log_buff, 0, LOG_BUFF_SIZE);
}

#if UART_PRINT_DEBUG_ENABLE
_attribute_ram_code_ static uint16_t get_log_queue_len() {
   return (log_head - log_tail) & (LOG_BUFF_MASK);
}

_attribute_ram_code_ static uint16_t get_log_freespace() {
    return (sizeof(log_buff)/sizeof(log_buff[0]) - get_log_queue_len());
}

_attribute_ram_code_ static void log_read(uint8_t **str, uint16_t len) {

    if (str && len) {
        for(int i = 0; i < len; i++) {
            *(*str+i) = log_buff[log_tail++];
            log_tail &= LOG_BUFF_MASK;
        }
    }
}

_attribute_ram_code_ static void log_write(uint8_t c) {

    if (!get_log_freespace()) {
        log_tail++;
        log_tail &= LOG_BUFF_MASK;
    }
    log_buff[log_head++] = c;
    log_head &= LOG_BUFF_MASK;
}

#endif /* UART_PRINT_DEBUG_ENABLE */

_attribute_ram_code_ void set_log_str() {

#if UART_PRINT_DEBUG_ENABLE
    memset(log_notify.str, 0, LOG_STR_SIZE);
    uint16_t len = get_log_queue_len();

    if (len > LOG_STR_SIZE) {
        len = LOG_STR_SIZE-1;
    }

    uint8_t *str = log_notify.str;

    log_read(&str, len);
    log_notify.pid++;
    log_notify.pid &= 0xFF;
    lg_notify = NOTIFY_MAX;
#endif /* UART_PRINT_DEBUG_ENABLE */

 }

void init_log() {
    log_notify.id = LOG_ID;
    log_notify.pid = 1;
    log_buff_clear();
#if UART_PRINT_DEBUG_ENABLE
    log_notify.debug_enabled = ON;
#else
    log_notify.debug_enabled = OFF;
#endif /* UART_PRINT_DEBUG_ENABLE */
//    send_log_enable = false;
}

#if 1
#if UART_PRINT_DEBUG_ENABLE

#ifndef     BIT_INTERVAL
#define     BIT_INTERVAL        (16000000/PRINT_BAUD_RATE)
#endif


_attribute_ram_code_
int uart_putc(char byte) //GPIO simulate uart print func
{
    unsigned char  j = 0;
    unsigned int t1 = 0,t2 = 0;

    //REG_ADDR8(0x582+((DEBUG_INFO_TX_PIN>>8)<<3)) &= ~(DEBUG_INFO_TX_PIN & 0xff) ;//Enable output

    unsigned int  pcTxReg = (0x583+((DEBUG_INFO_TX_PIN>>8)<<3));//register GPIO output
    unsigned char tmp_bit0 = read_reg8(pcTxReg) & (~(DEBUG_INFO_TX_PIN & 0xff));
    unsigned char tmp_bit1 = read_reg8(pcTxReg) | (DEBUG_INFO_TX_PIN & 0xff);
    unsigned char bit[10] = {0};

    bit[0] = tmp_bit0;
    bit[1] = (byte & 0x01)? tmp_bit1 : tmp_bit0;
    bit[2] = ((byte>>1) & 0x01)? tmp_bit1 : tmp_bit0;
    bit[3] = ((byte>>2) & 0x01)? tmp_bit1 : tmp_bit0;
    bit[4] = ((byte>>3) & 0x01)? tmp_bit1 : tmp_bit0;
    bit[5] = ((byte>>4) & 0x01)? tmp_bit1 : tmp_bit0;
    bit[6] = ((byte>>5) & 0x01)? tmp_bit1 : tmp_bit0;
    bit[7] = ((byte>>6) & 0x01)? tmp_bit1 : tmp_bit0;
    bit[8] = ((byte>>7) & 0x01)? tmp_bit1 : tmp_bit0;
    bit[9] = tmp_bit1;

    //unsigned char r = irq_disable();
    t1 = read_reg32(0x740);
    for(j = 0;j<10;j++)
    {
        t2 = t1;
        while(t1 - t2 < BIT_INTERVAL){
            t1  = read_reg32(0x740);
        }
        write_reg8(pcTxReg,bit[j]);        //send bit0
    }
    //irq_restore(r);

    return byte;
}
#endif
#endif

int putchar(int c) {
#if UART_PRINT_DEBUG_ENABLE
    log_write((uint8_t)c);
    return uart_putc((char)c);
//    return c;

#else
    return c;
#endif /* UART_PRINT_DEBUG_ENABLE */
}

