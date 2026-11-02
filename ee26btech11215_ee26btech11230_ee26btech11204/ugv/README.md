# Wi-Fi-Controlled Unmanned Ground Vehicle (UGV)

## Project Overview

This project focuses on the design and development of an Unmanned Ground Vehicle (UGV) using microcontrollers, wireless communication, and DC motor control.

Three different control models were explored:

1. **ESP32 Bluetooth-Controlled UGV** – A rover controlled wirelessly through Bluetooth using the Dabble application.
2. **Raspberry Pi Pico W Wi-Fi-Controlled UGV** – A rover controlled through a locally hosted web interface using Wi-Fi.
3. **ESP32-CAM-Based UGV** – A proposed camera-equipped rover designed around the ESP32-CAM platform.

The project also explores the concept of **Dynamic Node Hopping** as a possible extension for improving wireless communication coverage.

## Project Models

### 1. ESP32 Bluetooth-Controlled UGV

- Microcontroller: ESP32
- Communication: Bluetooth
- Control interface: Dabble application
- Motor driver: L293D
- Drive system: Two DC gear motors
- Features: Wireless directional control and adjustable motor speed

### 2. Raspberry Pi Pico W Wi-Fi-Controlled UGV

- Microcontroller: Raspberry Pi Pico W
- Communication: Wi-Fi
- Control interface: Web-based joystick
- Motor driver: L293D
- Drive system: Two DC gear motors
- Features: Local Wi-Fi access point, browser-based control, differential motor control, and automatic stopping when control updates are lost

### 3. ESP32-CAM-Based UGV

- Microcontroller: ESP32-CAM
- Communication: Wi-Fi
- Motor driver: L293D
- Drive system: Two DC gear motors
- Intended feature: Wireless camera-based monitoring and rover control

**Implementation status:** The ESP32-CAM model was developed at the theoretical and design level. Camera initialization encountered a hardware compatibility issue, and a successful live-streaming demonstration was not achieved.

## Repository Contents

```text
UGV-Project/
│
├── README.md
│
├── references.txt
│
├── Circuit_Diagrams/
│   ├── ESP32_Bluetooth_UGV
│   ├── Pico_W_WiFi_UGV
│   └── ESP32_CAM_UGV
│
├── Source_Code/
│   ├── ESP32_Bluetooth/
│   ├── Pico_W_WiFi/
│   └── ESP32_CAM/
│
└── Demonstrations/
    ├── ESP32_Bluetooth/
    └── Pico_W_WiFi/
```

*The directory structure above is a suggested organization; adjust the folder names to match the files actually included.*

## Hardware Components

- ESP32 development board
- Raspberry Pi Pico W
- ESP32-CAM module
- L293D motor driver
- DC geared motors
- Rechargeable battery supply
- Connecting wires and supporting components

## Wireless Communication

The project explores two primary wireless control approaches:

- Bluetooth communication for direct control through a mobile application.
- Wi-Fi communication for browser-based control through a locally hosted web server.

The Pico W implementation uses HTTP requests to transmit joystick coordinates to the rover. These coordinates are processed to generate differential motor commands.

## Dynamic Node Hopping

Dynamic Node Hopping is considered as a possible extension to the project. The concept involves using intermediate wireless nodes to extend communication coverage between the operator and the UGV.

This remains a proposed research direction rather than a completed and experimentally validated feature.

## References

Technical documentation, circuit diagrams, demonstration videos, and supporting research sources are listed in [`references.txt`](references.txt).

## Project Status

| Model | Development status |
|---|---|
| ESP32 Bluetooth UGV | Developed and demonstrated |
| Pico W Wi-Fi UGV | Developed and demonstrated |
| ESP32-CAM UGV | Theoretical design; physical demonstration incomplete |
| Dynamic Node Hopping | Proposed extension |

## Acknowledgements

This project was carried out under the guidance of **Prof. Dr. GVV Sharma**.

**Project contributors:**
- Nehan Mohammad
- T. Pranay Siddarth Reddy
- Akshit
