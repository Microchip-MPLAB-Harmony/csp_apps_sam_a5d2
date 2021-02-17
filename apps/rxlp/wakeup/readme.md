---
parent: Harmony 3 peripheral library application examples for SAMA5D2 family
title: RXLP wakeup 
has_children: false
has_toc: false
---

[![MCHP](https://www.microchip.com/ResourcePackages/Microchip/assets/dist/images/logo.png)](https://www.microchip.com)

# RXLP wakeup

This example application shows how RXLP can wakeup the device.

## Description

Application starts by flashing the LED to indicate that it is powered up and running. Upon pressing the switch, it shutdowns the device(stops toggling the LED). When a character is sent over RXLP UART (from PC), the device will reset and run the application (starts toggling the LEDs).

## Downloading and building the application

To clone or download this application from Github, go to the [main page of this repository](https://github.com/Microchip-MPLAB-Harmony/csp_apps_sam_a5d2) and then click Clone button to clone this repository or download as zip file.
This content can also be downloaded using content manager by following these [instructions](https://github.com/Microchip-MPLAB-Harmony/contentmanager/wiki).

Path of the application within the repository is **apps/rxlp/wakeup/firmware** .

To build the application, refer to the following table and open the project using its IDE.

| Project Name      | Description                                    |
| ----------------- | ---------------------------------------------- |
| sam_a5d2_xult.X | MPLABX project for [SAMA5D2 Xplained Ultra Evaluation Kit](https://www.microchip.com/DevelopmentTools/ProductDetails/ATSAMA5D2C-XULT) |
| sam_a5d2_xult_iar.IAR/rxlp_wakeup_sam_a5d2_xult_iar.eww | IAR project for [SAMA5D2 Xplained Ultra Evaluation Kit](https://www.microchip.com/DevelopmentTools/ProductDetails/ATSAMA5D2C-XULT) |
|||

## Setting up AT91Bootstrap loader

To load the application binary onto the target device, we need to use at91bootstrap loader. Refer to the [at91bootstrap loader documentation](../../docs/readme_bootstrap.md) for details on how to configure, build and run bootstrap loader project and use it to bootstrap the application binaries.

## Setting up the hardware

The following table shows the target hardware for the application projects.

| Project Name| Board|
|:---------|:---------:|
| sam_a5d2_xult.X <br> sam_a5d2_xult_iar.IAR/rxlp_wakeup_sam_a5d2_xult_iar.eww | [SAMA5D2 Xplained Ultra Evaluation Kit](https://www.microchip.com/DevelopmentTools/ProductDetails/ATSAMA5D2C-XULT) |
|||

### Setting up [SAMA5D2 Xplained Ultra Evaluation Kit](https://www.microchip.com/DevelopmentTools/ProductDetails/ATSAMA5D2C-XULT)

#### Addtional hardware required

- SD Card with FAT32 file system

#### Setting up the SD Card

- Download harmony MPU bootstrap loader from this [location](firmware/at91bootstrap_sam_a5d2_xult.X/binaries/boot.bin)
- Copy the downloaded boot loader binary( boot.bin) onto the SD card

#### Setting up the board

- SDMMC slot used for bootloading the application is SDMMC1
- Short jumper JP2 (DEBUG_DIS)
- Connect the Debug USB port on the board to the computer using a micro USB cable
- Connect the USB-UART click board to the computer using a mini USB cable
- Connect the USB-UART click board to the SAMA5D2-XULT board with the following pin connections:

    | SAMA5D2-XULT pins | USB-UART CLICK Pins |
    | ---------- | -------------|
    | J12, PIN 9 | TX |
    | J12, PIN 11 | GND |
    |||

## Running the Application

1. Build the application using its IDE
2. Copy the output binary (named 'harmony.bin') onto the SD Card (Refer to the 'Setting up hardware' section above for setting up the SD card)
3. Insert the SD card into SDMMC slot on the board (Refer to the 'Setting up hardware' section for the correct SDMMC slot)
4. Reset the board to run the application
5. LED toggles indicating the board is up and running
6. Press switch  to shutdown the device (LED toggling stops)
7. Configure USB-UART virtual COM port to operate with the following parameters (9600 baud, 8 bit data, no parity, 1 stop bit)
7. Send a character on the USB UART Virtual COM port to reset the device
8. Device resets (indicated by the LED starting to toggle again)

Following table provides details of switch and LED:

| Board      | Switch Name |  LED Name |
| ---------- | ----------- | --------- |
| [SAMA5D2 Xplained Ultra Evaluation Kit](https://www.microchip.com/DevelopmentTools/ProductDetails/ATSAMA5D2C-XULT) |PB_USER |RGB_LED(Blue)  |
||||
