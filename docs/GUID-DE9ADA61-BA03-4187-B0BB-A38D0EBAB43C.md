# Cache Maintenance

This example application shows how to do cache maintenance operation by cleaning and<br /> invalidating the cache for the DMA buffers located in the cacheable SRAM region.

**Parent topic:**[Harmony 3 Peripheral Library Application Examples for SAMA5D2 Family](GUID-3730E5D6-911C-4BCA-9955-26D7EB66B585.md)

## Description

This example application shows how to use the cache maintenance APIs to avoid issues<br /> related to cache coherency when the data cache is enabled. Application uses USART<br /> and the DMA PLIBs to demonstrate cache maintenance APIs provided by the Cache<br /> peripheral library.

Application registers a callback with the DMA transmit and receive channels. It then<br /> transmits a message using the DMA transmit channel and then schedules a read of ten<br /> characters using the DMA receive channel. Once the DMA reads complete, it reads the<br /> received data and echoes the same on the terminal using the DMA transmit<br /> channel.

Application calls DCACHE\_CLEAN\_BY\_ADDR API on the write buffer before transmitting<br /> it. Calling this API copies the data from the cache memory to the main memory,<br /> thereby ensuring that the DMA peripheral uses the updated values in the write<br /> buffer.

Application calls the DCACHE\_INVALIDATE\_BY\_ADDR API on the read buffer after<br /> reception of data is complete by the DMA receive channel. Calling this API<br /> invalidates the cache region corresponding to the read buffer, thereby ensuring that<br /> the CPU reads updated values in the main memory into the read buffer \(and into the<br /> cache memory\).

Cache maintenance operations are always performed on a cache line \(1 cache line = 32<br /> bytes\), thus the read and write buffers must be aligned to a 32 byte boundary and<br /> must be a multiple of 32 bytes. This is the reason why 32 byte buffers are used for<br /> receiving and echoing 10 bytes of data.

## Downloading and Building the Application

To clone or download this application from Github, go to the [main page of this repository](https://github.com/Microchip-MPLAB-Harmony/csp_apps_sam_a5d2) and then<br /> click Clone button to clone this repository or download as zip file. This content<br /> can also be downloaded using content manager by following these [instructions](https://github.com/Microchip-MPLAB-Harmony/contentmanager/wiki).

Path of the application within the repository is<br /> **apps/cache/cache\_maintenance/firmware**.

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

-   Connect the UART Debug port \(J28\) on the board to the computer using a FTDI USB to serial cable.
-   Connect the Debug USB port on the board to the computer using a Type-C USB cable

## Running the Application

1.  Build the application using its IDE
2.  Open the Terminal application \(Ex.:Tera term\) on the computer.
3.  Connect to the Virtual COM port and configure the serial settings as follows:
    -   Baud : 115200
    -   Data : 8 Bits
    -   Parity : None
    -   Stop : 1 Bit
    -   Flow Control : None
4.  The console displays the following message

    ![](GUID-4757B177-844A-4371-BAD2-E607E742AAAC-low.png)

5.  Type 10 characters in the terminal. It will echo back the received bytes and toggles the LED

The following table provides the LED name:

|Board|LED Name|
|-----|--------|
|[SAMA5D29 Curiosity Development Board](https://www.microchip.com/en-us/development-tool/EV07R15A)|RGB\_LED \(Blue\)|

