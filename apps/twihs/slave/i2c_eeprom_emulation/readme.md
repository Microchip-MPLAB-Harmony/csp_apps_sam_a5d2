---
parent: Harmony 3 peripheral library application examples for SAMA5D2 family
title: TWIHS EEPROM emulation
has_children: false
has_toc: false
---

[![MCHP](https://www.microchip.com/ResourcePackages/Microchip/assets/dist/images/logo.png)](https://www.microchip.com)

# TWIHS EEPROM emulation

This example application demonstrates how to use the TWIHS peripheral in slave mode.

## Description

This example uses the TWIHS peripheral library in slave mode and emulates an EEPROM of 512 bytes. There are two pages each of size 256 bytes. TWIHS slave expects two bytes of memory address from the TWIHS master and the memory address can range from 0x00 to 0x1FF.
TWIHS slave application supports following:

**Byte Write**: The TWIHS master sends the slave address, followed by two bytes of memory address. The slave provides the data present at the requested memory address.

**Page Write**: A page write is initiated the same way as a byte write, but the TWIHS master can write up-to 256 bytes (1 page). If more than 256 bytes are sent by the TWIHS master to the TWIHS slave, the memory address will “roll over” and previous data will be overwritten. The address “roll over” during write is from the last byte of the current page to the first byte of the same page.

**Current Address Read**: The internal memory address counter maintains the last address accessed during the last read or write operation, incremented by one. Once the device address with the read/write bit set to one is clocked in and acknowledged by the EEPROM, the data byte at the current address is serially clocked out. After reading the data from the current address, the TWIHS master sends NAK and generates a STOP condition.

**Random Read**: The TWIHS master writes the 2 byte memory address and then reads the data from that memory address. After reading the data, the TWIHS master sends NAK and generates a STOP condition.

**Sequential Read**: Sequential reads are initiated by either a current address read or a random address read. As long as the EEPROM receives an acknowledge, it will continue to increment the memory address and serially clock out sequential data bytes. When the memory address limit is reached (0x1FF), the memory address will “roll over” to 0x00 and the sequential read will continue. The sequential read operation is terminated when the TWIHS master sends a NAK and generates a STOP condition.

To run the application, two evaluation kits will be required - one acting as the TWIHS master and the other as the TWIHS slave. The TWIHS master application to be used for this demo application is available under **apps/twihs/master/twihs_eeprom/firmware** folder. TWIHS master writes an array of values to the TWIHS slave and verifies the value written by reading the values back and comparing it to the value written.

## Downloading and building the application

To clone or download this application from Github, go to the [main page of this repository](https://github.com/Microchip-MPLAB-Harmony/csp_apps_sam_a5d2) and then click Clone button to clone this repository or download as zip file.
This content can also be downloaded using content manager by following these [instructions](https://github.com/Microchip-MPLAB-Harmony/contentmanager/wiki).

Path of the application within the repository is **apps/twihs/slave/i2c_eeprom_emulation/firmware** .

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
| sam_a5d2_curiosity.X | [SAMA5D2 Curiosity]() |
|||

## Setting up the hardware

The following table shows the target hardware for the application projects.

| Project Name| Board|
|:---------|:---------:|
| sam_a5d2_curiosity.X | [SAMA5D2 Curiosity]() |
|||

Hardware setup requires two boards, one acting as the TWIHS Master and the other as the TWIHS slave.

### Setting up [SAMA5D2 Curiosity]()

#### Setting up the board

- Connect the Debug USB port on the board to the computer using a micro USB cable
- connect TWIHS SDA line on Pin 11 (PD4) of J18 connector and TWIHS SCL line on Pin 12 (PD5) with the corresponding SDA and SCL lines of the TWIHS master. Refer the TWIHS master documentation for connection details.
- Connect a ground wire between TWIHS master and TWIHS slave boards

## Running the Application

1. Open the twihs_eeprom application from *apps/twihs/master/twihs_eeprom/firmware* and open the *main.c* file. Make sure the following macro is defined to *false*.
   ```
   #define I2C_SLAVE_IS_ONBOARD_EEPROM           false
   ```

   Build twihs_eeprom application (which is used as TWIHS master) using its IDE

2. Build TWIHS slave application (used as TWIHS slave) using its IDE
3. Run application on TWIHS slave board and then run the application on TWIHS master board
4. LED on the TWIHS master board indicates success or failure:

    - The LED is turned ON when the value read from the TWIHS slave matches with the written value
    - The LED is turned OFF when the value read from the TWIHS slave did not match with the written value
