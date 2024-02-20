---
parent: Harmony 3 peripheral library application examples for SAMA5D2 family
title: SPI ping pong with DMA 
has_children: false
has_toc: false
---

[![MCHP](https://www.microchip.com/ResourcePackages/Microchip/assets/dist/images/logo.png)](https://www.microchip.com)

# SPI ping pong with DMA

This example application shows how to continuously transmit and receive data over a SPI interface using ping pong buffers with the DMA.

## Description

DMA peripheral provides support to implement ping-pong buffering. DMA transmits and receives data from one pair of buffers, when CPU works on the second pair.

When DMA is transmitting data from ping buffer, CPU works on pong buffer. When DMA completes transfer, it generates an interrupt to indicate transfer completion from ping buffer. DMA channel is automatically re-initialized to transmit data from pong buffer while the CPU works on the ping buffer.

When DMA is receiving data to ping buffer, CPU works on pong buffer. When DMA completes transfer, it generates an interrupt to indicate the availability of data in the ping buffer. DMA channel is automatically re-initialized to receive data in the pong buffer while the CPU works on the ping buffer.

Data is transmitted and received in loopback mode by shorting the MISO and MOSI lines. At the end of transfer, received data is compared with transmitted data to confirm data match.

## Downloading and building the application

To clone or download this application from Github, go to the [main page of this repository](https://github.com/Microchip-MPLAB-Harmony/csp_apps_sam_a5d2) and then click Clone button to clone this repository or download as zip file.
This content can also be downloaded using content manager by following these [instructions](https://github.com/Microchip-MPLAB-Harmony/contentmanager/wiki).

Path of the application within the repository is **apps/spi/spi_ping_pong_with_dma/firmware** .

To build the application, refer to the following table and open the project using its IDE.

| Project Name      | Description                                    |
| ----------------- | ---------------------------------------------- |
| sam_a5d2_curiosity.X | MPLABX project for [SAMA5D29 Curiosity Development Board](https://www.microchip.com/en-us/development-tool/EV07R15A) |
|||

## Setting up AT91Bootstrap loader

To load the application binary onto the target device, we need to use at91bootstrap loader. Refer to the [at91bootstrap loader documentation](../../docs/readme_bootstrap.md) for details on how to configure, build and run bootstrap loader project and use it to bootstrap the application binaries.

## Setting up the hardware

The following table shows the target hardware for the application projects.

| Project Name| Board|
|:---------|:---------:|
| sam_a5d2_curiosity.X  | [SAMA5D29 Curiosity Development Board](https://www.microchip.com/en-us/development-tool/EV07R15A) |
|||

### Setting up [SAMA5D29 Curiosity Development Board](https://www.microchip.com/en-us/development-tool/EV07R15A)

#### Setting up the board

- Connect the Debug USB port on the board to the computer using a micro USB cable
- Use a jumper wire to short pin 5 and pin 6 of mikroBUS 1 connector

## Running the Application

1. Build the application using its IDE
2. LED indicates the success or failure:
    - The LED is turned ON when the value read matched with the written value
    - The LED is turned OFF when the value read did not match with the written value

Refer to the following table for LED name:

| Board      | LED Name                                    |
| ----------------- | ---------------------------------------------- |
| [SAMA5D29 Curiosity Development Board](https://www.microchip.com/en-us/development-tool/EV07R15A) | RGB_LED(Green)  |
|||