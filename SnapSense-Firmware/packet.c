#include "packet.h"
#include "uart.h"
#include <string.h>

/* simple XOR checksum */
uint8_t packet_checksum(const uint8_t *payload, uint8_t len)
{
    uint8_t csum = 0;

    for (uint8_t i = 0; i < len; i++) {
        csum ^= payload[i];
    }

    return csum;
}

/* send packet through UART */
void packet_send(uint8_t type,
                 const uint8_t *payload,
                 uint8_t len)
{
    uint8_t csum = packet_checksum(payload, len);

    uart_send_byte(PKT_SOF);

    uart_send_byte(type);

    uart_send_byte(len);

    uart_send_buffer(payload, len);

    uart_send_byte(csum);

    uart_send_byte(PKT_EOF);
}

/* parser init */
void packet_parser_init(PacketParser *parser)
{
    parser->state = STATE_WAIT_SOF;

    parser->payload_index = 0;

    parser->rx_checksum = 0;

    memset(&parser->pkt, 0, sizeof(Packet));
}

/* parse incoming UART bytes */
uint8_t packet_parse_byte(PacketParser *parser,
                          uint8_t byte,
                          Packet *out)
{
    switch (parser->state) {

    case STATE_WAIT_SOF:

        if (byte == PKT_SOF) {
            parser->state = STATE_READ_TYPE;
        }

        break;

    case STATE_READ_TYPE:

        parser->pkt.type = byte;

        parser->state = STATE_READ_LENGTH;

        break;

    case STATE_READ_LENGTH:

        if (byte > PKT_MAX_PAYLOAD) {

            parser->state = STATE_WAIT_SOF;

            break;
        }

        parser->pkt.length = byte;

        parser->payload_index = 0;

        if (byte == 0) {

            parser->state = STATE_READ_CHECKSUM;

        } else {

            parser->state = STATE_READ_PAYLOAD;
        }

        break;

    case STATE_READ_PAYLOAD:

        parser->pkt.payload[parser->payload_index] = byte;

        parser->payload_index++;

        if (parser->payload_index ==
            parser->pkt.length) {

            parser->state = STATE_READ_CHECKSUM;
        }

        break;

    case STATE_READ_CHECKSUM:

        parser->rx_checksum = byte;

        parser->state = STATE_WAIT_EOF;

        break;

    case STATE_WAIT_EOF:

        if (byte == PKT_EOF) {

            uint8_t expected =
                packet_checksum(parser->pkt.payload,
                                parser->pkt.length);

            if (expected == parser->rx_checksum) {

                *out = parser->pkt;

                parser->state = STATE_WAIT_SOF;

                return 1;
            }
        }

        parser->state = STATE_WAIT_SOF;

        break;

    default:

        parser->state = STATE_WAIT_SOF;

        break;
    }

    return 0;
}