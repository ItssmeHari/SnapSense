#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>

/* GPIO setup */
void gpio_init(void);

/* ultrasonic sensor */
uint32_t ultrasonic_read_cm(void);

/* LED functions */
void led_on(void);

void led_off(void);

void led_toggle(void);

/* buzzer functions */
void buzzer_on(void);

void buzzer_off(void);

/* small delay */
void delay_us(uint32_t us);

#endif