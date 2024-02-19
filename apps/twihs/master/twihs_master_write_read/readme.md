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

Path of the application within the repository is **apps/twihs/master/twihs_eeprom/firmware** .

To build the application, refer to the following table and open the project using its IDE.

| Project Name      | Description                                    |
| ----------------- | ---------------------------------------------- |
| sam_a5d2_curiosity.X | MPLABX project for [SAMA5D2 Curiosity]() |
|||

## Setting up AT91Bootstrap loader

To load the application binary onto the target device, we need to use at91bootstrap loader. Refer to the [at91bootstrap loader documentation](../../../docs/readme_bootstrap.md) for details on how to configure, build and run bootstrap loader project and use it to bootstrap the application binaries.

## Setting up the hardware

The following table shows the target hardware for the application projects.

| Project Name| Board|
|:---------|:---------:|
| sam_a5d2_curiosity.X  | [SAMA5D2 Curiosity]() |
|||

### Setting up [SAMA5D2 Curiosity]()

#### Setting up the board

- Connect the Debug USB port on the board to the computer using a micro USB cable
- If this application is used for communicating with *apps/twihs/slave/i2c_eeprom_emulation* application, then connect TWIHS SDA line on Pin 11 (PD4) of J19 connector and TWIHS SCL line on Pin 12 (PD5) of J19 connector with the corresponding SDA and SCL lines of the TWIHS slave.

## Running the Application

1. Open the main.c file and make sure the following macro is defined to *true* when communicating with on-board EEPROM and to *false* when communicating with the *apps/twihs/slave/i2c_eeprom_emulation* application.

   When communicating with on-board EEPROM:
   
   `#define I2C_SLAVE_IS_ONBOARD_EEPROM           true`

   When communicating with *apps/twihs/slave/i2c_eeprom_emulation* application

   `#define I2C_SLAVE_IS_ONBOARD_EEPROM           false`

2. Build the application using its IDE

3. The LED indicates the success or failure:
    - The LED is turned ON when the value read from the EEPROM matched with the written value
    - The LED is turned OFF when the value read from the EEPROM did not match with the written value

Refer to the following table for LED name:

| Board      | LED Name                                    |
| ----------------- | ---------------------------------------------- |
| [SAMA5D2 Curiosity]() |RGB_LED(Green)  |
|||
