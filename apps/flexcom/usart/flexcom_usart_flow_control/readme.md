---
parent: Harmony 3 peripheral library application examples for SAMA5D2 family
title: FLEXCOM USART interrupt 
has_children: false
has_toc: false
---

[![MCHP](https://www.microchip.com/ResourcePackages/Microchip/assets/dist/images/logo.png)](https://www.microchip.com)

# FLEXCOM USART flow control

This example application shows how to use the flexcom module in USART flow control mode.

## Description

This application configures FLEXCOM0 in non-blocking mode and FLEXCOM1 in blocking mode. FLEXCOM0 is used for data transmission and FLEXCOM1 is used for receiption. The TX line of FLEXCOM0 is connected to the RX line of FLEXCOM1. The RTS line of FLEXCOM1 is connected to the CTS line of FLEXCOM0. FLEXCOM0 transmits 100 characters using interrupts. Application however simulates a slow receiver by inserting a delay after each character is read on FLEXCOM1. As a result, once the RX FIFO is full, the RTS line is de-asserted and transmitter (FLEXCOM0) stops sending the data. Eventually, when the receiver reads out sufficient number of bytes from the RX FIFO, such that the
number of bytes in the FIFO goes below RXFTHRES2, the RTS line is again asserted and the transmitter continues sending the data. The demo application indicates success if the received data matches the transmitted data.

## Downloading and building the application

To clone or download this application from Github, go to the [main page of this repository](https://github.com/Microchip-MPLAB-Harmony/csp_apps_sam_a5d2) and then click Clone button to clone this repository or download as zip file.
This content can also be downloaded using content manager by following these [instructions](https://github.com/Microchip-MPLAB-Harmony/contentmanager/wiki).

Path of the application within the repository is **apps/flexcom/usart/flexcom_usart_flow_control/firmware** .

To build the application, refer to the following table and open the project using its IDE.

| Project Name      | Description                                    |
| ----------------- | ---------------------------------------------- |
| sam_a5d2_curiosity.X | MPLABX project for [SAMA5D29 Curiosity Development Board](https://www.microchip.com/en-us/development-tool/EV07R15A) |
|||

## Setting up AT91Bootstrap loader

To load the application binary onto the target device, we need to use at91bootstrap loader. Refer to the [at91bootstrap loader documentation](../../../docs/readme_bootstrap.md) for details on how to configure, build and run bootstrap loader project and use it to bootstrap the application binaries.

## Setting up the hardware

The following table shows the target hardware for the application projects.

| Project Name| Board|
|:---------|:---------:|
| sam_a5d2_curiosity.X | [SAMA5D29 Curiosity Development Board](https://www.microchip.com/en-us/development-tool/EV07R15A) |
|||

### Setting up [SAMA5D29 Curiosity Development Board](https://www.microchip.com/en-us/development-tool/EV07R15A)


#### Setting up the board

- Connect the UART Debug port on the board to the computer using a FTDI to USB cable
- Make connections as following:
    - Short pin 5 (FLEXCOM1_TXD) of J8 connector to pin 21 (FLEXCOM4_RXD) of J25 connector.
    - Short pin 6 (FLEXCOM1 CTS) of J8 connector to pin 26 (FLEXCOM4 RTS) of J25 connector.

## Running the Application

1. Build the application using its IDE
2. LED is turned ON on success.
3. The following table provides the LED names

    | Board      | LED Name                                    |
    | ----------------- | ---------------------------------------------- |
    | [SAMA5D29 Curiosity Development Board](https://www.microchip.com/en-us/development-tool/EV07R15A) | RGB_LED(Green) |
    |||
