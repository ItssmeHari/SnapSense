#include "cmd.h"
#include "packet.h"
#include "adc.h"
#include "gpio.h"
#include <stdint.h>

/* updated every 100ms in main loop */
extern volatile uint32_t g_uptime_ticks;

/* read temp from LM35 and send back */
void cmd_read_temperature(const Packet *req)
{
    (void)req;

    int16_t temp_x10 = adc_read_temperature_x10();
    
    uint8_t payload[2];

    payload[0] = (uint8_t)((temp_x10 >> 8) & 0xFF);
    payload[1] = (uint8_t)(temp_x10 & 0xFF);

    packet_send(PKT_TYPE_TEMPERATURE, payload, 2);
}

/* ultrasonic distance */
void cmd_read_distance(const Packet *req)
{
    (void)req;

    uint32_t dist_cm = ultrasonic_read_cm();

    uint8_t payload[2];

    payload[0] = (uint8_t)((dist_cm >> 8) & 0xFF);
    payload[1] = (uint8_t)(dist_cm & 0xFF);

    packet_send(PKT_TYPE_DISTANCE, payload, 2);
}

/* LED control */
void cmd_led_on(const Packet *req)
{
    (void)req;

    led_on();

    uint8_t payload[1] = {0x01};

    packet_send(PKT_TYPE_STATUS, payload, 1);
}

void cmd_led_off(const Packet *req)
{
    (void)req;

    led_off();

    uint8_t payload[1] = {0x00};

    packet_send(PKT_TYPE_STATUS, payload, 1);
}

/* buzzer control */
void cmd_buzzer_on(const Packet *req)
{
    (void)req;

    buzzer_on();

    uint8_t payload[1] = {0x01};

    packet_send(PKT_TYPE_STATUS, payload, 1);
}

void cmd_buzzer_off(const Packet *req)
{
    (void)req;
    
    buzzer_off();

    uint8_t payload[1] = {0x00};

    packet_send(PKT_TYPE_STATUS, payload, 1);
}

/* send uptime value */
void cmd_get_status(const Packet *req)
{
    (void)req;

    uint32_t t = g_uptime_ticks;

    uint8_t payload[4];

    payload[0] = (uint8_t)((t >> 24) & 0xFF);
    payload[1] = (uint8_t)((t >> 16) & 0xFF);
    payload[2] = (uint8_t)((t >> 8) & 0xFF);
    payload[3] = (uint8_t)(t & 0xFF);

    packet_send(PKT_TYPE_STATUS, payload, 4);
}

/* command map */
static const CommandEntry dispatch_table[] = {
    { PKT_TYPE_TEMPERATURE, cmd_read_temperature },
    { PKT_TYPE_DISTANCE,    cmd_read_distance    },
    { 0x03,                 cmd_led_on           },
    { PKT_TYPE_LED_OFF,     cmd_led_off          },
    { PKT_TYPE_BUZZER_ON,   cmd_buzzer_on        },
    { PKT_TYPE_BUZZER_OFF,  cmd_buzzer_off       },
    { PKT_TYPE_CMD,         cmd_get_status       },
};

#define DISPATCH_TABLE_SIZE \
    (sizeof(dispatch_table) / sizeof(dispatch_table[0]))

/* check cmd and call handler */
void dispatch_command(const Packet *request)
{
    for (uint8_t i = 0; i < DISPATCH_TABLE_SIZE; i++) 
    {
        if (dispatch_table[i].cmd_id == request->type) 
        {
            dispatch_table[i].handler(request);
            return;
        }
    }

    /* Unknown command — send error response */
    uint8_t err_payload[1] = { request->type };

    packet_send(PKT_TYPE_ERROR, err_payload, 1);
}