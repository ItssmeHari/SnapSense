#include "stm32f10x.h"
#include "uart.h"
#include "adc.h"
#include "gpio.h"
#include "packet.h"
#include "cmd.h"

/* system uptime counter */
volatile uint32_t g_uptime_ticks = 0;

/* simple ms delay */
static void delay_ms(uint32_t ms)
{
    while (ms--) {
        delay_us(1000);
    }
}

/* read sensors and send packets */
static void send_sensor_data(void)
{
    /* temperature */
    int16_t temp_x10 = adc_read_temperature_x10();

    uint8_t temp_payload[2];

    temp_payload[0] = (uint8_t)((temp_x10 >> 8) & 0xFF);
    temp_payload[1] = (uint8_t)(temp_x10 & 0xFF);

    packet_send(PKT_TYPE_TEMPERATURE, temp_payload, 2);

    /* distance */
    uint32_t dist_cm = ultrasonic_read_cm();

    uint8_t dist_payload[2];

    dist_payload[0] = (uint8_t)((dist_cm >> 8) & 0xFF);
    dist_payload[1] = (uint8_t)(dist_cm & 0xFF);

    packet_send(PKT_TYPE_DISTANCE, dist_payload, 2);

    /* status blink */
    led_toggle();
}

int main(void)
{
    /* peripheral setup */
    gpio_init();
    adc_init();
    uart_init();

    /* enable interrupts */
    __enable_irq();

    /* parser init */
    PacketParser parser;
    packet_parser_init(&parser);

    /* timing values */
    uint32_t last_sensor_tick = 0;

    const uint32_t SENSOR_INTERVAL_MS = 500;

    while (1) {

        /* send sensor values periodically */
        if ((g_uptime_ticks - last_sensor_tick)
             >= SENSOR_INTERVAL_MS) {

            last_sensor_tick = g_uptime_ticks;

            send_sensor_data();
        }

        /* UART packet handling */
        uint8_t byte;

        uint8_t bytes_processed = 0;

        while (bytes_processed < 16 &&
               uart_read_byte(&byte)) {

            Packet received_pkt;

            if (packet_parse_byte(&parser,
                                  byte,
                                  &received_pkt)) {

                dispatch_command(&received_pkt);
            }

            bytes_processed++;
        }

        /* update tick */
        g_uptime_ticks++;

        delay_ms(1);
    }

    return 0;
}