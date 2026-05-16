#ifndef ADC_H
#define ADC_H

#include <stdint.h>

/* ADC setup */
void adc_init(void);

/* raw ADC read */
uint16_t adc_read_raw(void);

/* LM35 temp value */
int16_t adc_read_temperature_x10(void);

#endif