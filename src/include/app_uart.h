#ifndef SRC_INCLUDE_APP_UART_H_
#define SRC_INCLUDE_APP_UART_H_

#define UART_DEBUG     ON
#define UART_DATA_LEN  188

typedef struct _uart_data_t {
        uint32_t dma_len;        // dma len must be 4 byte
        uint8_t  data[UART_DATA_LEN];
} uart_data_t;

void app_uart_init();
void app_uart_irq_proc();
size_t send_to_uart(const uint8_t *data, size_t len);
size_t response_from_uart(uint8_t *data, size_t len);
size_t get_data_len_from_uart();

#endif /* SRC_INCLUDE_APP_UART_H_ */
