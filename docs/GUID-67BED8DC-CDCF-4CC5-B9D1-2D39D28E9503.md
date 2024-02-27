# FLEXCOM SPI EEPROM Read Write

This example application shows how to use the FLEXCOM module in SPI mode.

**Parent topic:**[Harmony 3 Peripheral Library Application Examples for SAMA5D2 Family](GUID-3730E5D6-911C-4BCA-9955-26D7EB66B585.md)

## Description

This application configures the FLEXCOM peripheral in SPI mode to read and write data<br /> from an external EEPROM memory chip. Data is written to the EEPROM and then read<br /> back and compared. Successful comparison is indicated by turning on an LED.

## Downloading and Building the Application

To clone or download this application from Github, go to the [main page of this repository](https://github.com/Microchip-MPLAB-Harmony/csp_apps_sam_a5d2) and then<br /> click Clone button to clone this repository or download as zip file. This content<br /> can also be downloaded using content manager by following these [instructions](https://github.com/Microchip-MPLAB-Harmony/contentmanager/wiki).

Path of the application within the repository is<br /> **apps/flexcom/spi/master/flexcom\_spi\_eeprom/firmware**.

To build the application, refer to the following table and open the project using its<br /> IDE.

|Project Name|Description|
|------------|-----------|
|sam\_a5d2\_curiosity.X|MPLABX project for [SAMA5D29 Curiosity Development Board Kit](https://www.microchip.com/en-us/development-tool/EV07R15A)|

## Setting Up AT91Bootstrap Loader

To load the application binary onto the target device, we need to use at91bootstrap<br /> loader. Refer to the [at91bootstrap loader documentation](GUID-DA6B998E-C5DD-4566-BB08-7DC124553FBF.md) for<br /> details on how to configure, build and run bootstrap loader project and use it to<br /> bootstrap the application binaries.

## Setting Up the Hardware

The following table shows the target hardware for the application projects.

|Project Name|Description|
|------------|-----------|
|sam\_a5d2\_curiosity.X|[SAMA5D29 Curiosity Development Board Kit](https://www.microchip.com/en-us/development-tool/EV07R15A)|

### Setting Up SAMA5D29 Curiosity Development Board

**ADDITIONAL HARDWARE REQUIRED**

-   [EEPROM 4 Click board](https://www.mikroe.com/eeprom-4-click)

**SETTING UP THE BOARD**

-   Connect the Debug USB port on the board to the computer using a Type-C USB cable
-   Connect [EEPROM 4 Click board](https://www.mikroe.com/eeprom-4-click) to RPi 40-pin connector \(J25\) connector on the board as per below Pin connections:

    |RPi 40-pin connector<br /> \(J25\) Pins|EEPROM 4 CLICK<br /> Pins|
    |-----------------------------------------------------|---------------------------------------|
    |PIN 16|HLD|
    |PIN 15|WP|
    |PIN 24|CS|
    |PIN 23|SCK|
    |PIN 19|SDI|
    |PIN 21|SDO|
    |PIN 39|GND|
    |PIN 1|3V3|


## Running the Application

1.  Build the application using its IDE
2.  LED indicates the success or failure:
    -   LED is turned ON when the value read from the EEPROM matched with the written value
    -   LED is turned OFF when the value read from the EEPROM did not match with the written value

Following table provides the LED name:

|Board|LED Name|
|-----|--------|
|[SAMA5D29 Curiosity Development Board](https://www.microchip.com/en-us/development-tool/EV07R15A)|RGB\_LED \(Green\)|

