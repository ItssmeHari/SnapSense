# SnapSense Firmware

SnapSense is a bare-metal firmware project for the STM32F103C8T6 (Blue Pill) that reads sensor data and sends it to a PC using a custom UART packet protocol.

The project uses:
- LM35 temperature sensor
- HC-SR04 ultrasonic sensor
- UART communication
- Ring buffer
- State machine packet parser
- Function pointer command system

---

# Features

- Bare-metal STM32 programming
- UART binary packet communication
- Temperature and distance sensing
- LED and buzzer control from PC
- Interrupt-driven UART RX
- Modular firmware structure

---

# Hardware

| Component | Pin |
|---|---|
| LM35 | PA0 |
| HC-SR04 TRIG | PA1 |
| HC-SR04 ECHO | PA2 |
| LED | PC13 |
| Buzzer | PB0 |
| USART1 TX | PA9 |
| USART1 RX | PA10 |

---

# Packet Format
[SOF][TYPE][LEN][PAYLOAD][CHECKSUM][EOF]

| Field | Value |
| ----- | ----- |
| SOF   | 0xAA  |
| EOF   | 0x55  |

Example temperature packet:
AA 01 02 01 00 01 55

---

# Project Structure

snapsense-firmware/
├── main.c
├── uart.c/h
├── adc.c/h
├── gpio.c/h
├── packet.c/h
└── cmd.c/h

---

# Concepts Used

* Ring buffer for UART RX
* State machine parser
* Function pointer dispatch table
* Register-level STM32 programming

---

# Purpose

This project was built to learn:

* Embedded systems
* STM32 bare-metal programming
* UART protocols
* Interrupt handling
* Firmware architecture

No HAL library is used.
