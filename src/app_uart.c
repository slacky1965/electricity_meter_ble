#include <stdint.h>
#include "tl_common.h"
#include "drivers.h"
#include "drivers/8258/uart.h"

#include "app_uart.h"

_attribute_data_retention_ uart_data_t rec_buff = {0,  {0, } };
_attribute_data_retention_ uart_data_t trans_buff = {0,   {0, } };


void app_uart_init()
{
    //note: dma addr must be set first before any other uart initialization! (confirmed by sihui)
    uart_recbuff_init( (unsigned char *)&rec_buff, sizeof(rec_buff));

    uart_gpio_set(UART_TX_GPIO, UART_RX_GPIO);

    uart_reset();  //will reset uart digital registers from 0x90 ~ 0x9f, so uart setting must set after this reset

    //baud rate: 9600
    #if (CLOCK_SYS_CLOCK_HZ == 16000000)
        uart_init(118, 13, PARITY_NONE, STOP_BIT_ONE);
    #elif (CLOCK_SYS_CLOCK_HZ == 24000000)
        uart_init(249, 9, PARITY_NONE, STOP_BIT_ONE);
    #endif


    uart_dma_enable(1, 1);  //uart data in hardware buffer moved by dma, so we need enable them first

    irq_set_mask(FLD_IRQ_DMA_EN);
    dma_chn_irq_enable(FLD_DMA_CHN_UART_RX | FLD_DMA_CHN_UART_TX, 1);       //uart Rx/Tx dma irq enable

    uart_irq_enable(0, 0);      //uart Rx/Tx irq no need, disable them

    irq_enable();

}

_attribute_ram_code_ void app_uart_irq_proc() {

    unsigned char uart_dma_irqsrc;
    //1. UART irq
    uart_dma_irqsrc = dma_chn_irq_status_get();///in function,interrupt flag have already been cleared,so need not to clear DMA interrupt flag here


    if(uart_dma_irqsrc & FLD_DMA_CHN_UART_RX){
        dma_chn_irq_status_clr(FLD_DMA_CHN_UART_RX);

        //Received uart data in rec_buff, user can copy data here

    }
    if(uart_dma_irqsrc & FLD_DMA_CHN_UART_TX){
        dma_chn_irq_status_clr(FLD_DMA_CHN_UART_TX);
    }

}

_attribute_ram_code_ size_t send_to_uart(const uint8_t *data, size_t len) {

    if (len > UART_DATA_LEN) len = UART_DATA_LEN;

    memcpy(trans_buff.data, data, len);
    trans_buff.dma_len = len;

    if (uart_dma_send((uint8_t*)&trans_buff)) {
#if UART_PRINT_DEBUG_ENABLE && UART_DEBUG
        printf("send bytes: %u\r\n", len);
#endif
        return len;
    }

    return 0;
}

_attribute_ram_code_ size_t response_from_uart(uint8_t *data, size_t len) {

    if (len > UART_DATA_LEN) len = UART_DATA_LEN;

    if (rec_buff.dma_len) {
        if (len  <= rec_buff.dma_len) {
            memcpy(data, rec_buff.data, len);
            if (len < rec_buff.dma_len) {
                memcpy(rec_buff.data, rec_buff.data+len, rec_buff.dma_len - len);
            }
            rec_buff.dma_len -= len;
        } else {
            memcpy(data, rec_buff.data, rec_buff.dma_len);
            len = rec_buff.dma_len;
            rec_buff.dma_len = 0;
        }
#if UART_PRINT_DEBUG_ENABLE && UART_DEBUG
        printf("resp bytes: %u\r\n", len);
#endif
        return len;
    }

    return 0;
}

_attribute_ram_code_ size_t get_data_len_from_uart() {
    return rec_buff.dma_len;
}

_attribute_ram_code_ void flush_uart_buff() {
    memset(&rec_buff, 0, sizeof(uart_data_t));
}
