---
parent: Harmony 3 peripheral library application examples for SAMA5D2 family
title: TC compare mode 
has_children: false
has_toc: false
---

[![MCHP](https://www.microchip.com/ResourcePackages/Microchip/assets/dist/images/logo.png)](https://www.microchip.com)

# TC compare mode

This example application shows how to use the TC module in compare mode to generate an active low, active high, and toggle output on compare match.

## Description

Three TC channels are configured in compare mode. Each channel generates different output waveform depending upon configured action on compare match and period match.

**Active low output**: Output is set high on compare match A and is set low on compare match C.

**Active high output**: Output is set low on compare match A and is set high on compare match C.

**Toggle output**: Output toggles on compare match A.

## Downloading and building the application

To clone or download this application from Github, go to the [main page of this repository](https://github.com/Microchip-MPLAB-Harmony/csp_apps_sam_a5d2) and then click Clone button to clone this repository or download as zip file.
This content can also be downloaded using content manager by following these [instructions](https://github.com/Microchip-MPLAB-Harmony/contentmanager/wiki).

Path of the application within the repository is **apps/tc/tc_compare_mode/firmware** .

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

## Running the Application

1. Build the application using its IDE
2. Observe generated waveforms on the oscilloscope

### [SAMA5D2 Curiosity]()

| Pin      | Observable characteristic of the waveform |
| ---------| -----------------------------------------|
| PC3 (Pin 5 of mikroBUS1) | Active high output with 1 Hz frequency |
| PC4 (Pin 3 of mikroBUS1) | Active low output with 1 Hz frequency |
|||
