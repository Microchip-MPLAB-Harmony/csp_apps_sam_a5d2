---
parent: Harmony 3 peripheral library application examples for SAMA5D2 family
title: MCAN interrupt 
has_children: false
has_toc: false
---

[![MCHP](https://www.microchip.com/ResourcePackages/Microchip/assets/dist/images/logo.png)](https://www.microchip.com)

# MCAN interrupt

This example application shows how to use the MCAN module to transmit and receive CAN messages in interrupt mode.

## Description

This application transmits and receives CAN messages on the CAN bus. To run this application, two evaluation boards of same type are required. These boards acts as different nodes on the CAN bus. Same application is programmed onto both the boards. Boards are connected to PC via UART. While running the application, user can send and receive CAN messages between the boards using UART console applications running on the PC.

## Downloading and building the application

To clone or download this application from Github, go to the [main page of this repository](https://github.com/Microchip-MPLAB-Harmony/csp_apps_sam_a5d2) and then click Clone button to clone this repository or download as zip file.
This content can also be downloaded using content manager by following these [instructions](https://github.com/Microchip-MPLAB-Harmony/contentmanager/wiki).

Path of the application within the repository is **apps/mcan/mcan_normal_operation_interrupt/firmware** .

To build the application, refer to the following table and open the project using its IDE.

| Project Name      | Description                                    |
| ----------------- | ---------------------------------------------- |
| sam_a5d2_curiosity.X | MPLABX project for [SAMA5D2 Curiosity]() |
|||

## Setting up AT91Bootstrap loader

To load the application binary onto the target device, we need to use at91bootstrap loader. Refer to the [at91bootstrap loader documentation](../../docs/readme_bootstrap.md) for details on how to configure, build and run bootstrap loader project and use it to bootstrap the application binaries.

## Setting up the hardware

The following table shows the target hardware for the application projects.

| Project Name| Board|
|:---------|:---------:|
| sam_a5d2_curiosity.X | [SAMA5D2 Curiosity]() |
|||

### Setting up [SAMA5D2 Curiosity]()

#### Addtional hardware required

- To run the demo, the following additional hardware are required:
  - [SAMA5D2 Curiosity]() | 2 |
  - [ATA6563 click board](https://www.mikroe.com/ata6563-click)	| 2 |

#### Setting up the board

- Connect  to [ATA6563 click board](https://www.mikroe.com/ata6563-click) using female to female DB9 serial cable
- Connect each [ATA6563 click board](https://www.mikroe.com/ata6563-click) to SAM A5D2 CURIOSITY board as per the Pin connections shown below

    | SAMA5D2 CURIOSITY | ATA6563 click board |
    | ------------ | ------------------- |
    | CANTX0 (PIN4), mikroBUS1 Connector | RX (PIN 13) |
    | CANRX0 (PIN6), mikroBUS1 Connector | TX (PIN 14) |
    | 3V3 (PIN20), mikroBUS1 Connector   | 3V3 (PIN 7) |
    | VDD_5V (PIN3), mikroBUS1 Connector | 5V (PIN 10) |
    | GND (PIN19), mikroBUS1 Connector   | GND (PIN 9) |
    |||

- Connect the UART Debug port on the board to the computer using a FTDI to USB cable

## Running the Application

1. Build the application using its IDE
2. Open the Terminal application (Ex.:Tera term) on the computer.
3. Connect to the (USB to Serial) COM port and configure the serial settings as follows:
    - Baud : 115200
    - Data : 8 Bits
    - Parity : None
    - Stop : 1 Bit
    - Flow Control : None
4. Build and Program the application on both the boards using their respective IDEs
5. In the console associated with board 1, press "1" to transmit a CAN message
6. Transmitted message status will be displayed in the console window of board 1
7. Received message will be displayed in the console window of board 2
8. Follow the sequence below to send and receive CAN message:
    - Press '1' on board 2
9. If the steps are executed in this sequence, the final output in the consoles will be as below:

  ![output](images/mcan_normal_operation_interrupt.png)

