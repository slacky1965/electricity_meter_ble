#ifndef SRC_INCLUDE_APP_UART_H_
#define SRC_INCLUDE_APP_UART_H_

#define UART_DEBUG     ON
#define UART_DATA_LEN  188
#define UART_BUFF_SIZE 1024             /* size ring buffer  */
#define UART_BUFF_MASK UART_BUFF_SIZE-1 /* mask ring buffer  */

typedef struct _uart_data_t {
        uint32_t dma_len;        // dma len must be 4 byte
        uint8_t  data[UART_DATA_LEN];
} uart_data_t;

void app_uart_init();
void app_uart_irq_proc();
size_t write_bytes_to_uart(const uint8_t *data, size_t len);
uint8_t read_byte_from_buff_uart();
uint8_t available_buff_uart();
void flush_buff_uart();

#endif /* SRC_INCLUDE_APP_UART_H_ */
