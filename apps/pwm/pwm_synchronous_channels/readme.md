---
parent: Harmony 3 peripheral library application examples for SAMA5D2 family
title: PWM synchronous channels 
has_children: false
has_toc: false
---

[![MCHP](https://www.microchip.com/ResourcePackages/Microchip/assets/dist/images/logo.png)](https://www.microchip.com)

# PWM synchronous channels

This example application shows how to use the PWM peripheral to generate 3-phase PWM signals with dead time.

## Description

This example shows how to configure the PWM to generate synchronous 3-phase PWM signals with dead time (used for motor control). The duty cycle of the PWM is updated in the interrupt handler.

## Downloading and building the application

To clone or download this application from Github, go to the [main page of this repository](https://github.com/Microchip-MPLAB-Harmony/csp_apps_sam_a5d2) and then click Clone button to clone this repository or download as zip file.
This content can also be downloaded using content manager by following these [instructions](https://github.com/Microchip-MPLAB-Harmony/contentmanager/wiki).

Path of the application within the repository is **apps/pwm/pwm_synchronous_channels/firmware** .

To build the application, refer to the following table and open the project using its IDE.

| Project Name      | Description                                    |
| ----------------- | ---------------------------------------------- |
| sam_a5d2_curiosity.X | MPLABX project for [SAMA5D2 Curiosity]() |
|||

## Setting up AT91Bootstrap loader

To load the application binary onto the target device, we need to use at91bootstrap loader. Refer to the [at91bootstrap loader documentation](../../docs/readme_bootstrap.md) for details on how to configure, build and run bootstrap loader project and use it to bootstrap the application binaries.

## Setting up the hardware

The following table shows the target hardware for the application projects.

| Project Name| Board|
|:---------|:---------:|
| sam_a5d2_curiosity.X  | [SAMA5D2 Curiosity]() |
|||

### Setting up [SAMA5D2 Curiosity]()

#### Setting up the board

- Connect the Debug USB port on the board to the computer using a micro USB cable
- Connect PB0 (PIN 16 of mikroBUS2 (J22)) to an an oscilloscope/Logic analyzer

## Running the Application

1. Build the application using its IDE
2. Observe the high-side PWM waveform on the oscilloscope
3. PWM frequency is 20 KHz
4. Duty cycle varies from 0 to 100 %

Refer to the below table for PWM output pins for different boards:

| PWM Channel | [SAMA5D2 Curiosity]() |
| ---------|---------------------- |
| CH1_PWMH | PIN 16 of mikroBUS2 (J22) |
|||