# TRNG Random Number

This example application shows how to use the TRNG Peripheral library to generate and<br /> read a random number.

**Parent topic:**[Harmony 3 Peripheral Library Application Examples for SAMA5D2 Family](GUID-3730E5D6-911C-4BCA-9955-26D7EB66B585.md)

## Description

This example application shows how to use the TRNG peripheral library to produce a<br /> 32-bit true random number. Generated random number can be used as an entropy source<br /> for seeding a Deterministic Random Number Generator \(DRNG\), creating cryptographic<br /> keys for ciphers, etc.

## Downloading and Building the Application

To clone or download this application from Github, go to the [main page of this repository](https://github.com/Microchip-MPLAB-Harmony/csp_apps_sam_a5d2) and then<br /> click Clone button to clone this repository or download as zip file. This content<br /> can also be downloaded using content manager by following these [instructions](https://github.com/Microchip-MPLAB-Harmony/contentmanager/wiki).

Path of the application within the repository is<br /> **apps/trng/random\_number/firmware**.

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

**SETTING UP THE BOARD**

-   Connect the Debug USB port on the board to the computer using a Type-C USB cable

## Running the Application

1.  Build the application using its IDE
2.  The console displays the following message

    ![](GUID-DE7E3B0B-EB04-4F1C-BE2C-7EE5E9DC6D91-low.png)

3.  Every time the switch is pressed, a new 32 bit random number is generated and displayed on the console

    ![](GUID-24BC65EF-6343-4241-AD53-2FE6BE14D514-low.png)


Following table provides the Switch name:

|Board|Switch Name|
|-----|-----------|
|[SAMA5D29 Curiosity Development Board](https://www.microchip.com/en-us/development-tool/EV07R15A)|USER|

