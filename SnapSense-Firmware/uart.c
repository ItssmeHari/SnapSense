#include "uart.h"
#include "stm32f10x.h"

/* RX ring buffer */
RingBuffer rx_ring = { .head = 0, .tail = 0 };

/* UART setup */
void uart_init(void)
{
    /* enable clocks */
    RCC->APB2ENR |= (1 << 2)
                 |  (1 << 14)
                 |  (1 << 0);

    /* PA9 -> TX */
    GPIOA->CRH &= ~(0xF << 4);
    GPIOA->CRH |=  (0xB << 4);

    /* PA10 -> RX */
    GPIOA->CRH &= ~(0xF << 8);
    GPIOA->CRH |=  (0x4 << 8);

    /* USART config */
    USART1->BRR = 0x271;

    USART1->CR1 = (1 << 3)
                | (1 << 2)
                | (1 << 5)
                | (1 << 13);

    /* enable USART1 interrupt */
    NVIC->ISER[1] |= (1 << (37 - 32));
}

/* send one byte */
void uart_send_byte(uint8_t byte)
{
    while (!(USART1->SR & (1 << 7))) {}

    USART1->DR = byte;
}

/* send buffer */
void uart_send_buffer(const uint8_t *buf,
                      uint8_t len)
{
    for (uint8_t i = 0; i < len; i++) {

        uart_send_byte(buf[i]);
    }
}

/* read byte from RX buffer */
uint8_t uart_read_byte(uint8_t *out)
{
    if (rx_ring.head == rx_ring.tail) {

        return 0;
    }

    *out = rx_ring.buffer[rx_ring.tail];

    rx_ring.tail =
        (rx_ring.tail + 1) % RING_BUFFER_SIZE;

    return 1;
}

/* check if data exists */
uint8_t uart_data_available(void)
{
    return (rx_ring.head != rx_ring.tail) ? 1 : 0;
}

/* UART RX interrupt */
void USART1_IRQHandler(void)
{
    if (USART1->SR & (1 << 5)) {

        uint8_t byte =
            (uint8_t)USART1->DR;

        uint8_t next_head =
            (rx_ring.head + 1)
             % RING_BUFFER_SIZE;

        if (next_head != rx_ring.tail) {

            rx_ring.buffer[rx_ring.head] = byte;

            rx_ring.head = next_head;
        }
    }
}