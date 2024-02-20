---
parent: Harmony 3 peripheral library application examples for SAMA5D2 family
title: ADC user sequence 
has_children: false
has_toc: false
---

[![MCHP](https://www.microchip.com/ResourcePackages/Microchip/assets/dist/images/logo.png)](https://www.microchip.com)

# ADC user sequence

This example application shows how to sample three analog inputs using the user sequencer in software trigger mode and send the converted data to the console.

## Description

Conversion of the three analog inputs is triggered from software trigger at every 500 ms. The user sequencer is used to define the order of conversion:

| Board | Sequence of conversion |
| ----- | ---------------------- |
| [SAMA5D29 Curiosity Development Board](https://www.microchip.com/en-us/development-tool/EV07R15A) |  AD5 --> AD6 --> AD0 |
|||

Analog input voltage in the range of 0 V to 3.3 V is fed to the ADC input channel AD5, AD6 and AD0. ADC conversion result is displayed on the console.

## Downloading and building the application

To clone or download this application from Github, go to the [main page of this repository](https://github.com/Microchip-MPLAB-Harmony/csp_apps_sam_a5d2) and then click Clone button to clone this repository or download as zip file.
This content can also be downloaded using content manager by following these [instructions](https://github.com/Microchip-MPLAB-Harmony/contentmanager/wiki).

Path of the application within the repository is **apps/adc/adc_user_sequence/firmware** .

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

- Connect the UART Debug port (J28) on the board to the computer using a FTDI USB to serial cable.
- Use a jumper wire to connect Pin 37 of J25 (AD5 is mapped to Port Pin PD24) to 3.3 V or GND
- Use a jumper wire to connect Pin 1 of mikroBUS1 J19 (AD6 is mapped to Port Pin PD25) to 3.3 V or GND
- Use a jumper wire to connect Pin 1 of mikroBUS2 J22 (AD7 is mapped to Port Pin PD26) to 3.3 V or GND

## Running the Application

1. Build the application using its IDE
2. Open the Terminal application (Ex.:Tera term) on the computer.
3. Connect to the FTDI Virtual COM port and configure the serial settings as follows:
    - Baud : 115200
    - Data : 8 Bits
    - Parity : None
    - Stop : 1 Bit
    - Flow Control : None
4. The console displays the ADC Count and the ADC Input Voltage

![output](images/output_adc_user_sequence.png)

