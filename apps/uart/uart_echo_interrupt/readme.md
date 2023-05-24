---
parent: Harmony 3 peripheral library application examples for SAMA5D2 family
title: UART echo interrupt 
has_children: false
has_toc: false
---

[![MCHP](https://www.microchip.com/ResourcePackages/Microchip/assets/dist/images/logo.png)](https://www.microchip.com)

# UART echo interrupt

This example application shows how to use the UART peripheral to transfer a block of data in a non-blocking manner.

## Description

This example shows the read and write operation over a UART in a non-blocking manner. The peripheral interrupt is used to manage the transfer. It receives 10 characters from the terminal window and echoes them back.

## Downloading and building the application

To clone or download this application from Github, go to the [main page of this repository](https://github.com/Microchip-MPLAB-Harmony/csp_apps_sam_a5d2) and then click Clone button to clone this repository or download as zip file.
This content can also be downloaded using content manager by following these [instructions](https://github.com/Microchip-MPLAB-Harmony/contentmanager/wiki).

Path of the application within the repository is **apps/uart/uart_echo_interrupt/firmware** .

To build the application, refer to the following table and open the project using its IDE.

| Project Name      | Description                                    |
| ----------------- | ---------------------------------------------- |
| sam_a5d2_xult.X | MPLABX project for [SAMA5D2 Xplained Ultra Evaluation Kit](https://www.microchip.com/DevelopmentTools/ProductDetails/ATSAMA5D2C-XULT) |
| sam_a5d27_som1_ek.X | MPLABX project for [SAMA5D27-SOM1-EK1](https://www.microchip.com/DevelopmentTools/ProductDetails/atsama5d27-som1-ek1) |
|||

## Setting up AT91Bootstrap loader

To load the application binary onto the target device, we need to use at91bootstrap loader. Refer to the [at91bootstrap loader documentation](../../docs/readme_bootstrap.md) for details on how to configure, build and run bootstrap loader project and use it to bootstrap the application binaries.

## Setting up the hardware

The following table shows the target hardware for the application projects.

| Project Name| Board|
|:---------|:---------:|
| sam_a5d2_xult.X  | [SAMA5D2 Xplained Ultra Evaluation Kit](https://www.microchip.com/DevelopmentTools/ProductDetails/ATSAMA5D2C-XULT) |
| sam_a5d27_som1_ek.X | [SAMA5D27-SOM1-EK1](https://www.microchip.com/DevelopmentTools/ProductDetails/atsama5d27-som1-ek1) |
|||

### Setting up [SAMA5D2 Xplained Ultra Evaluation Kit](https://www.microchip.com/DevelopmentTools/ProductDetails/ATSAMA5D2C-XULT)

#### Setting up the board

- Short jumper JP2 (DEBUG_DIS)
- Connect the Debug USB port on the board to the computer using a micro USB cable

### Setting up [SAMA5D27-SOM1-EK1](https://www.microchip.com/DevelopmentTools/ProductDetails/atsama5d27-som1-ek1)

#### Setting up the board

- Connect the Debug USB port on the board to the computer using a micro USB cable

## Running the Application

1. Build the application using its IDE
2. Open the Terminal application (Ex.:Tera term) on the computer.
3. Connect to the EDBG/Jlink Virtual COM port and configure the serial settings as follows:
    - Baud : 115200
    - Data : 8 Bits
    - Parity : None
    - Stop : 1 Bit
    - Flow Control : None

4. In console window, type 10 characters
5. Entered characters will be echoed back and the LED is toggled

Refer to the following table for LED name:

| Board      | LED Name                                    |
| ----------------- | ---------------------------------------------- |
| [SAMA5D2 Xplained Ultra Evaluation Kit](https://www.microchip.com/DevelopmentTools/ProductDetails/ATSAMA5D2C-XULT) |RGB_LED(Green)  |
| [SAMA5D27-SOM1-EK1](https://www.microchip.com/DevelopmentTools/ProductDetails/atsama5d27-som1-ek1) | D5(Green) |
|||
