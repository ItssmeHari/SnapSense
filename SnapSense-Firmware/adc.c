#include "adc.h"
#include "stm32f10x.h"

/* small delay for ADC startup */
static void delay_loop(uint32_t count)
{
    while (count--) {
        __NOP();
    }
}

/* ADC setup */
void adc_init(void)
{
    /* enable GPIOA + ADC1 clock */
    RCC->APB2ENR |= (1 << 2)
                 |  (1 << 9);

    /* PA0 analog mode */
    GPIOA->CRL &= ~(0xF << 0);

    /* sample time for channel 0 */
    ADC1->SMPR2 &= ~(0x7 << 0);
    ADC1->SMPR2 |=  (0x5 << 0);

    /* single conversion on channel 0 */
    ADC1->SQR3 = 0;
    ADC1->SQR1 = 0;

    /* ADC on */
    ADC1->CR2 |= (1 << 0);

    delay_loop(100);

    /* reset calibration */
    ADC1->CR2 |= (1 << 3);
    while (ADC1->CR2 & (1 << 3)) {}

    /* start calibration */
    ADC1->CR2 |= (1 << 2);
    while (ADC1->CR2 & (1 << 2)) {}
}

/* raw ADC value */
uint16_t adc_read_raw(void)
{
    /* start conversion */
    ADC1->CR2 |= (1 << 0);

    /* wait till conversion complete */
    while (!(ADC1->SR & (1 << 1))) {}

    return (uint16_t)(ADC1->DR & 0x0FFF);
}

/* LM35 temp read */
int16_t adc_read_temperature_x10(void)
{
    uint16_t raw = adc_read_raw();

    /* convert ADC value to mV */
    uint32_t voltage_mV =
        ((uint32_t)raw * 3300UL) / 4096UL;

    return (int16_t)voltage_mV;
}