#ifndef PACKET_H
#define PACKET_H

#include <stdint.h>

#define PKT_SOF          0xAA
#define PKT_EOF          0x55
#define PKT_MAX_PAYLOAD  8

/* packet IDs */
#define PKT_TYPE_TEMPERATURE  0x01
#define PKT_TYPE_DISTANCE     0x02
#define PKT_TYPE_STATUS       0x03
#define PKT_TYPE_LED_ON       0x03
#define PKT_TYPE_LED_OFF      0x04
#define PKT_TYPE_BUZZER_ON    0x05
#define PKT_TYPE_BUZZER_OFF   0x06
#define PKT_TYPE_CMD          0xFF
#define PKT_TYPE_ERROR        0xEE

/* packet data */
typedef struct {

    uint8_t type;

    uint8_t length;

    uint8_t payload[PKT_MAX_PAYLOAD];

    uint8_t checksum;

} Packet;

/* parser states */
typedef enum {

    STATE_WAIT_SOF,

    STATE_READ_TYPE,

    STATE_READ_LENGTH,

    STATE_READ_PAYLOAD,

    STATE_READ_CHECKSUM,

    STATE_WAIT_EOF

} ParserState;

/* parser info */
typedef struct {

    ParserState state;

    Packet pkt;

    uint8_t payload_index;

    uint8_t rx_checksum;

} PacketParser;

/* send packet */
void packet_send(uint8_t type,
                 const uint8_t *payload,
                 uint8_t len);

/* parser function */
uint8_t packet_parse_byte(PacketParser *parser,
                          uint8_t byte,
                          Packet *out);

/* parser init */
void packet_parser_init(PacketParser *parser);

/* checksum helper */
uint8_t packet_checksum(const uint8_t *payload,
                        uint8_t len);

#endif