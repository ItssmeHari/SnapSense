#include "gpio.h"
#include "stm32f10x.h"

/* small microsecond delay */
void delay_us(uint32_t us)
{
    while (us--) {

        __NOP(); __NOP(); __NOP(); __NOP();
        __NOP(); __NOP(); __NOP(); __NOP();
        __NOP(); __NOP(); __NOP(); __NOP();
    }
}

/* GPIO setup */
void gpio_init(void)
{
    /* enable GPIOA, GPIOB, GPIOC clock */
    RCC->APB2ENR |= (1 << 2)
                 |  (1 << 3)
                 |  (1 << 4);

    /* PA1 -> TRIG output */
    GPIOA->CRL &= ~(0xF << 4);
    GPIOA->CRL |=  (0x2 << 4);

    /* PA2 -> ECHO input */
    GPIOA->CRL &= ~(0xF << 8);
    GPIOA->CRL |=  (0x4 << 8);

    /* PC13 -> onboard LED */
    GPIOC->CRH &= ~(0xF << 20);
    GPIOC->CRH |=  (0x2 << 20);

    /* LED OFF initially */
    GPIOC->ODR |= (1 << 13);

    /* PB0 -> buzzer */
    GPIOB->CRL &= ~(0xF << 0);
    GPIOB->CRL |=  (0x2 << 0);

    /* buzzer OFF initially */
    GPIOB->ODR &= ~(1 << 0);
}

/* ultrasonic sensor read */
uint32_t ultrasonic_read_cm(void)
{
    uint32_t timeout;
    uint32_t echo_us = 0;

    /* trigger pulse */
    GPIOA->ODR |=  (1 << 1);

    delay_us(10);

    GPIOA->ODR &= ~(1 << 1);

    /* wait for echo */
    timeout = 100000;

    while (!(GPIOA->IDR & (1 << 2))) {

        if (--timeout == 0)
            return 0;
    }

    /* measure echo time */
    timeout = 38000;

    while (GPIOA->IDR & (1 << 2)) {

        echo_us++;

        delay_us(1);

        if (--timeout == 0)
            break;
    }

    return echo_us / 58;
}

/* LED control */
void led_on(void)
{
    GPIOC->ODR &= ~(1 << 13);
}

void led_off(void)
{
    GPIOC->ODR |= (1 << 13);
}

void led_toggle(void)
{
    GPIOC->ODR ^= (1 << 13);
}

/* buzzer control */
void buzzer_on(void)
{
    GPIOB->ODR |= (1 << 0);
}

void buzzer_off(void)
{
    GPIOB->ODR &= ~(1 << 0);
}