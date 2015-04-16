#ifndef _BSP_UART_H_
#define _BSP_UART_H_


void bsp_uart_init(void);


int uart1_fifo_read(unsigned char *data, int size);
int uart1_fifo_write(unsigned char *data, int size);

#endif
