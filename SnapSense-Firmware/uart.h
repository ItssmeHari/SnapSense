#ifndef UART_H
#define UART_H

#include <stdint.h>

#define RING_BUFFER_SIZE 256

typedef struct {

    uint8_t buffer[RING_BUFFER_SIZE];

    uint8_t head;

    uint8_t tail;

} RingBuffer;

/* UART setup */
void uart_init(void);

/* send byte */
void uart_send_byte(uint8_t byte);

/* send multiple bytes */
void uart_send_buffer(const uint8_t *buf,
                      uint8_t len);

/* read from RX buffer */
uint8_t uart_read_byte(uint8_t *out);

/* check buffer */
uint8_t uart_data_available(void);

/* global RX buffer */
extern RingBuffer rx_ring;

#endif