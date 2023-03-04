#include <stdint.h>
#include "tl_common.h"
#include "drivers.h"
#include "drivers/8258/uart.h"

#include "app_uart.h"

#if (ELECTRICITY_TYPE == KASKAD_1_MT || ELECTRICITY_TYPE == MERCURY_206)
#define BAUDRATE    9600
#elif (ELECTRICITY_TYPE == KASKAD_11)
#define BAUDRATE    2400
#endif

_attribute_data_retention_ uart_data_t rec_buff = {0,  {0, } };
_attribute_data_retention_ uart_data_t trans_buff = {0,   {0, } };
_attribute_data_retention_ uint8_t     uart_buff[UART_BUFF_SIZE];
_attribute_data_retention_ uint16_t    uart_head, uart_tail;

_attribute_ram_code_ uint8_t available_buff_uart() {
    if (uart_head != uart_tail) {
        return true;
    }
    return false;
}

_attribute_ram_code_ size_t get_queue_len_buff_uart() {
   return (uart_head - uart_tail) & (UART_BUFF_MASK);
}

_attribute_ram_code_ static size_t get_freespace_buff_uart() {
    return (sizeof(uart_buff)/sizeof(uart_buff[0]) - get_queue_len_buff_uart());
}

_attribute_ram_code_ void flush_buff_uart() {
    uart_head = uart_tail = 0;
    memset(uart_buff, 0, UART_BUFF_SIZE);
}

_attribute_ram_code_ uint8_t read_byte_from_buff_uart() {
    uint8_t ch = uart_buff[uart_tail++];
    uart_tail &= UART_BUFF_MASK;
    return ch;

}

_attribute_ram_code_ static size_t write_bytes_to_buff_uart(uint8_t *data, size_t len) {

    size_t free_space = get_freespace_buff_uart();
    size_t put_len;

    if (free_space >= len) put_len = len;
    else put_len = free_space;

    for (int i = 0; i < put_len; i++) {
        uart_buff[uart_head++] = data[i];
        uart_head &= UART_BUFF_MASK;
    }

    return put_len;
}

void app_uart_init() {

    //note: dma addr must be set first before any other uart initialization! (confirmed by sihui)
    uart_recbuff_init( (unsigned char *)&rec_buff, sizeof(rec_buff));

    uart_gpio_set(UART_TX_GPIO, UART_RX_GPIO);

    uart_reset();  //will reset uart digital registers from 0x90 ~ 0x9f, so uart setting must set after this reset

//    //baud rate: 9600
//#if (CLOCK_SYS_CLOCK_HZ == 16000000)
//        uart_init(118, 13, PARITY_NONE, STOP_BIT_ONE);
//#elif (CLOCK_SYS_CLOCK_HZ == 24000000)
//        uart_init(249, 9, PARITY_NONE, STOP_BIT_ONE);
//#endif

    uart_init_baudrate(BAUDRATE, CLOCK_SYS_CLOCK_HZ, PARITY_NONE, STOP_BIT_ONE);

    uart_dma_enable(1, 1);  //uart data in hardware buffer moved by dma, so we need enable them first

    irq_set_mask(FLD_IRQ_DMA_EN);
    dma_chn_irq_enable(FLD_DMA_CHN_UART_RX | FLD_DMA_CHN_UART_TX, 1);       //uart Rx/Tx dma irq enable

    uart_irq_enable(0, 0);      //uart Rx/Tx irq no need, disable them

    irq_enable();

}

_attribute_ram_code_ void app_uart_irq_proc() {

    uint8_t uart_dma_irqsrc, write_len;
    //1. UART irq
    uart_dma_irqsrc = dma_chn_irq_status_get();///in function,interrupt flag have already been cleared,so need not to clear DMA interrupt flag here


    if(uart_dma_irqsrc & FLD_DMA_CHN_UART_RX){
        dma_chn_irq_status_clr(FLD_DMA_CHN_UART_RX);

        //Received uart data in rec_buff, user can copy data here
        write_len = write_bytes_to_buff_uart(rec_buff.data, rec_buff.dma_len);

        if (write_len != 0) {
            if (write_len == rec_buff.dma_len) {
                rec_buff.dma_len = 0;
            } else if (write_len < rec_buff.dma_len) {
                memcpy(rec_buff.data, rec_buff.data+write_len, rec_buff.dma_len-write_len);
                rec_buff.dma_len -= write_len;
            }
        }

    }
    if(uart_dma_irqsrc & FLD_DMA_CHN_UART_TX){
        dma_chn_irq_status_clr(FLD_DMA_CHN_UART_TX);
    }

}

_attribute_ram_code_ size_t write_bytes_to_uart(const uint8_t *data, size_t len) {

    if (len > UART_DATA_LEN) len = UART_DATA_LEN;

    memcpy(trans_buff.data, data, len);
    trans_buff.dma_len = len;

    if (uart_dma_send((uint8_t*)&trans_buff)) {
        return len;
    }

    return 0;
}

