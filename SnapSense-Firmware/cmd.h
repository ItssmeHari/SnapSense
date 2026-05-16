#ifndef CMD_H
#define CMD_H

#include "packet.h"

/* command handler function */
typedef void (*CmdHandler)(const Packet *request);

/* command table entry */
typedef struct {

    uint8_t cmd_id;

    CmdHandler handler;

} CommandEntry;

/* packet dispatcher */
void dispatch_command(const Packet *request);

/* command handlers */
void cmd_read_temperature(const Packet *req);

void cmd_read_distance(const Packet *req);

void cmd_led_on(const Packet *req);

void cmd_led_off(const Packet *req);

void cmd_buzzer_on(const Packet *req);

void cmd_buzzer_off(const Packet *req);

void cmd_get_status(const Packet *req);

#endif