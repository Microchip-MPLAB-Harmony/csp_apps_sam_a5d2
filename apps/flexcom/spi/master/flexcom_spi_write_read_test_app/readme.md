---
parent: Harmony 3 peripheral library application examples for SAMA5D2 family
title: FLEXCOM SPI Master read write test application 
has_children: false
has_toc: false
---

[![MCHP](https://www.microchip.com/ResourcePackages/Microchip/assets/dist/images/logo.png)](https://www.microchip.com)

# FLEXCOM SPI Master read write test application

This is a FLEXCOM SPI Master test application which is provided to demonstrate communication between SPI master and the corresponding SPI slave application available under -  apps/flexcom/spi/slave/flexcom_spi_write_read/firmware

## Description

This example uses the FLEXCOM SPI peripheral library in master mode and serves as a test application to demonstrate communication between SPI master and corresponding SPI slave application available at apps/flexcom/spi/slave/flexcom_spi_write_read/firmware.

The SPI master writes data by sending a write command followed by two bytes of memory address followed by the data to be written.

< WR_CMD > < ADDR_MSB > < ADDR_LSB > < DATA0 > ... < DATA n >

The SPI slave asserts the Busy line to indicate to the SPI master that it is busy. Once ready, the SPI slave de-asserts the Busy line. Once the SPI slave is ready, the SPI master reads the data by sending read command followed by two bytes of memory address and the number of bytes to read.

< RD_CMD > < ADDR_MSB > < ADDR_LSB > < NUM_BYTES >

The SPI slave responds by sending the data at the requested memory address. The SPI master compares the sent data with the received data and repeats the test if it matches.

## Downloading and building the application

To clone or download this application from Github, go to the [main page of this repository](https://github.com/Microchip-MPLAB-Harmony/csp_apps_sam_a5d2) and then click Clone button to clone this repository or download as zip file.
This content can also be downloaded using content manager by following these [instructions](https://github.com/Microchip-MPLAB-Harmony/contentmanager/wiki).

Path of the application within the repository is **apps/flexcom/spi/master/flexcom_spi_write_read_test_app/firmware** .

To build the application, refer to the following table and open the project using its IDE.

| Project Name      | Description                                    |
| ----------------- | ---------------------------------------------- |
| sam_a5d2_xult.X | MPLABX project for [SAMA5D2 Xplained Ultra Evaluation Kit](https://www.microchip.com/DevelopmentTools/ProductDetails/ATSAMA5D2C-XULT) |
|||

## Setting up AT91Bootstrap loader

To load the application binary onto the target device, we need to use at91bootstrap loader. Refer to the [at91bootstrap loader documentation](../../../../docs/readme_bootstrap.md) for details on how to configure, build and run bootstrap loader project and use it to bootstrap the application binaries.

## Setting up the hardware

The following table shows the target hardware for the application projects.

| Project Name| Board|
|:---------|:---------:|
| sam_a5d2_xult.X | [SAMA5D2 Xplained Ultra Evaluation Kit](https://www.microchip.com/DevelopmentTools/ProductDetails/ATSAMA5D2C-XULT) |
|||

### Setting up [SAMA5D2 Xplained Ultra Evaluation Kit](https://www.microchip.com/DevelopmentTools/ProductDetails/ATSAMA5D2C-XULT)

#### Addtional hardware required

- Two SD Card with FAT32 file system

#### Setting up the SD Card

- Download harmony MPU bootstrap loader from this [location](firmware/at91bootstrap_sam_a5d2_xult.X/binaries/boot.bin)
- Copy the downloaded boot loader binary( boot.bin) onto the each SD card

#### Setting up the board

- To run this demo two [SAMA5D2 Xplained Ultra Evaluation Kit](https://www.microchip.com/DevelopmentTools/ProductDetails/ATSAMA5D2C-XULT) are required. One will be progammed as SPI master and other will be programmed as SPI slave.
- SDMMC slot used for bootloading the application is SDMMC1 on each board
- Short jumper JP2 (DEBUG_DIS) on each board
- Connect Pin 10, 15, 16, 17 on EXT1 of SPI slave to Pin 10, 15, 16, 17 on EXT1 of SPI master 
    - Pin 10 - SPI Chip Select
    - Pin 15 - SCK
    - Pin 16 - MOSI
    - Pin 17 - MISO
- Connect Pin 9 on EXT1 of SPI slave to Pin 9 on EXT1 of SPI master. Pin 9 is configured as GPIO and serves as the slave busy pin.
- Connect the Debug USB port on the board to the computer using a micro USB cable

## Running the Application

1. Build and Program the SPI master application project using its IDE on board.
2. Build and Program the SPI slave application project using its IDE. Path of the SPI Slave application within the repository is **apps/flexcom/spi/slave/flexcom_spi_write_read/firmware** on another board. 
3. Copy the output binary (named 'harmony.bin') onto the each SD Card (Refer to the 'Setting up hardware' section above for setting up the SD card).
4. Insert the SD card into SDMMC slot on each board (Refer to the 'Setting up hardware' section for the correct SDMMC slot).
5. Reset each board to run the application
6. On the SPI master board, press switch to start reading and writing data
7. LED on SPI master board indicates the success or failure:
    - LED is turned ON when the data read from the SPI slave matches with the data written
    - LED is turned OFF when the data read from the SPI slave does not match with the data written

Following table provides the LED name:

| Board      | LED Name  | Switch Name |
| ---------- | --------- | ----------- |
| [SAMA5D2 Xplained Ultra Evaluation Kit](https://www.microchip.com/DevelopmentTools/ProductDetails/ATSAMA5D2C-XULT) | RGB_LED(Green) | PB_USER |
|||
