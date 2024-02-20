---
parent: Harmony 3 peripheral library application examples for SAMA5D2 family
title: TWIHS EEPROM read write
has_children: false
has_toc: false
---

[![MCHP](https://www.microchip.com/ResourcePackages/Microchip/assets/dist/images/logo.png)](https://www.microchip.com)

# TWIHS EEPROM read write

This example application shows how to use the TWIHS peripheral to write and read from the I2C serial EEPROM memory.

## Description

This example uses the TWIHS peripheral library to write an array of values to the I2C Serial EEPROM and verify the value written by reading the values back and comparing it to the value written.

## Downloading and building the application

To clone or download this application from Github, go to the [main page of this repository](https://github.com/Microchip-MPLAB-Harmony/csp_apps_sam_a5d2) and then click Clone button to clone this repository or download as zip file.
This content can also be downloaded using content manager by following these [instructions](https://github.com/Microchip-MPLAB-Harmony/contentmanager/wiki).

Path of the application within the repository is **apps/twihs/master/twihs_eeprom_read_write/firmware** .

To build the application, refer to the following table and open the project using its IDE.

| Project Name      | Description                                    |
| ----------------- | ---------------------------------------------- |
| sam_a5d2_curiosity.X | MPLABX project for [SAMA5D29 Curiosity Development Board](https://www.microchip.com/en-us/development-tool/EV07R15A) |
|||

## Setting up AT91Bootstrap loader

To load the application binary onto the target device, we need to use at91bootstrap loader. Refer to the [at91bootstrap loader documentation](../../../../docs/readme_bootstrap.md) for details on how to configure, build and run bootstrap loader project and use it to bootstrap the application binaries.

## Setting up the hardware

The following table shows the target hardware for the application projects.

| Project Name| Board|
|:---------|:---------:|
| sam_a5d2_curiosity.X | [SAMA5D29 Curiosity Development Board](https://www.microchip.com/en-us/development-tool/EV07R15A) |
|||

### Setting up [SAMA5D29 Curiosity Development Board](https://www.microchip.com/en-us/development-tool/EV07R15A)

#### Addtional hardware required

- [EEPROM 3 click Board](https://www.mikroe.com/eeprom-3-click)

#### Setting up the board

- Connect the Debug USB port on the board to the computer using a USB-C cable.
- Connect EEPROM Click 3 Board to XPRO EXT2 (J19) connector on the board as per below Pin Connections table

| J19 MikroBus1 Pins | EEPROM 3 CLICK Pins |
| -------------------- | ------------------- |
| PIN 11 (PD4) | SDA |
| PIN 12 (PD5) | SCL |
| PIN 19 | GND |
| PIN 20 | 3V3 |
|||

## Running the Application

1. Build the application using its IDE
2. LED indicates the success or failure:
    - LED is turned ON when the value read from the EEPROM matched with the written value
    - LED is turned OFF when the value read from the EEPROM did not match with the written value

Following table provides the LED name:

| Board      | LED Name                                    |
| ----------------- | ---------------------------------------------- |
| [SAMA5D29 Curiosity Development Board](https://www.microchip.com/en-us/development-tool/EV07R15A) |RGB_LED(Green)  |
|||
