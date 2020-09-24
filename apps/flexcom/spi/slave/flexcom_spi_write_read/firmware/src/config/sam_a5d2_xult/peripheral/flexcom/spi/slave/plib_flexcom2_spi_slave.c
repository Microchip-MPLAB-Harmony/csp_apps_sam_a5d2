/*******************************************************************************
  SPIFLEXCOM2 SPI PLIB

  Company:
    Microchip Technology Inc.

  File Name:
    plib_flexcom2_spi_slave.c

  Summary:
    SPIFLEXCOM2 SPI Slave PLIB Implementation File.

  Description:
    This file defines the interface to the FLEXCOM SPI peripheral library.
    This library provides access to and control of the associated
    peripheral instance.

  Remarks:
    None.

*******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2018 Microchip Technology Inc. and its subsidiaries.
*
* Subject to your compliance with these terms, you may use Microchip software
* and any derivatives exclusively with Microchip products. It is your
* responsibility to comply with third party license terms applicable to your
* use of third party software (including open source software) that may
* accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
* EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
* WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
*******************************************************************************/
// DOM-IGNORE-END

#include "plib_flexcom2_spi_slave.h"
#include "peripheral/pio/plib_pio.h"
#include <string.h>

#define FLEXCOM2_READ_BUFFER_SIZE            256
#define FLEXCOM2_WRITE_BUFFER_SIZE           256

static uint8_t FLEXCOM2_ReadBuffer[FLEXCOM2_READ_BUFFER_SIZE];
static uint8_t FLEXCOM2_WriteBuffer[FLEXCOM2_WRITE_BUFFER_SIZE];


// *****************************************************************************
// *****************************************************************************
// Section: SPIFLEXCOM2 SPI Implementation
// *****************************************************************************
// *****************************************************************************

/* Global object to save FLEXCOM SPI Exchange related data */
FLEXCOM_SPI_SLAVE_OBJECT flexcom2SpiObj;

void FLEXCOM2_SPI_Initialize( void )
{
    /* Set FLEXCOM SPI operating mode */
    FLEXCOM2_REGS->FLEX_MR = FLEX_MR_OPMODE_SPI;

    /* Disable and Reset the SPI*/
    FLEXCOM2_REGS->FLEX_SPI_CR = FLEX_SPI_CR_SPIDIS_Msk | FLEX_SPI_CR_SWRST_Msk;

    /* SPI is by default in Slave Mode, disable mode fault detection */
    FLEXCOM2_REGS->FLEX_SPI_MR = FLEX_SPI_MR_MODFDIS_Msk;

    /* Set up clock Polarity, data phase, Communication Width */
    FLEXCOM2_REGS->FLEX_SPI_CSR[0] = FLEX_SPI_CSR_CPOL(0) | FLEX_SPI_CSR_NCPHA(1) | FLEX_SPI_CSR_BITS_8_BIT;

    flexcom2SpiObj.rdInIndex = 0;
    flexcom2SpiObj.wrOutIndex = 0;
    flexcom2SpiObj.nWrBytes = 0;
    flexcom2SpiObj.errorStatus = FLEXCOM_SPI_SLAVE_ERROR_NONE;
    flexcom2SpiObj.callback = NULL ;
    flexcom2SpiObj.transferIsBusy = false ;

    /* Set the Busy Pin to ready state */
    PIO_PinWrite((PIO_PIN)PIO_PIN_PD13, 0);

    /* Enable Receive full and chip deselect interrupt */
    FLEXCOM2_REGS->FLEX_SPI_IER = (SPI_IER_RDRF_Msk | SPI_IER_NSSR_Msk);

    /* Enable SPIFLEXCOM2 */
    FLEXCOM2_REGS->FLEX_SPI_CR = FLEX_SPI_CR_SPIEN_Msk;
}

/* For 9-bit mode, the "size" must be specified in terms of 16-bit words */
size_t FLEXCOM2_SPI_Read(void* pRdBuffer, size_t size)
{
    size_t rdSize = size;
    size_t rdInIndex = flexcom2SpiObj.rdInIndex;

    if (rdSize > rdInIndex)
    {
        rdSize = rdInIndex;
    }

    memcpy(pRdBuffer, FLEXCOM2_ReadBuffer, rdSize);

    return rdSize;
}

/* For 9-bit mode, the "size" must be specified in terms of 16-bit words */
size_t FLEXCOM2_SPI_Write(void* pWrBuffer, size_t size )
{
    uint32_t intState = FLEXCOM2_REGS->FLEX_SPI_IMR;
    size_t wrSize = size;

    FLEXCOM2_REGS->FLEX_SPI_IDR = intState;

    if (wrSize > FLEXCOM2_WRITE_BUFFER_SIZE)
    {
        wrSize = FLEXCOM2_WRITE_BUFFER_SIZE;
    }

    memcpy(FLEXCOM2_WriteBuffer, pWrBuffer, wrSize);

    flexcom2SpiObj.nWrBytes = wrSize;
    flexcom2SpiObj.wrOutIndex = 0;

    while ((FLEXCOM2_REGS->FLEX_SPI_SR & FLEX_SPI_SR_TDRE_Msk) && (flexcom2SpiObj.wrOutIndex < flexcom2SpiObj.nWrBytes))
    {
        FLEXCOM2_REGS->FLEX_SPI_TDR = FLEXCOM2_WriteBuffer[flexcom2SpiObj.wrOutIndex++];
    }

    /* Restore interrupt enable state and also enable TDRE interrupt */
    FLEXCOM2_REGS->FLEX_SPI_IER = (intState | FLEX_SPI_IER_TDRE_Msk);

    return wrSize;
}

/* For 9-bit mode, the return value is in terms of 16-bit words */
size_t FLEXCOM2_SPI_ReadCountGet(void)
{
    return flexcom2SpiObj.rdInIndex;
}

/* For 9-bit mode, the return value is in terms of 16-bit words */
size_t FLEXCOM2_SPI_ReadBufferSizeGet(void)
{
    return FLEXCOM2_READ_BUFFER_SIZE;
}

/* For 9-bit mode, the return value is in terms of 16-bit words */
size_t FLEXCOM2_SPI_WriteBufferSizeGet(void)
{
    return FLEXCOM2_WRITE_BUFFER_SIZE;
}

void FLEXCOM2_SPI_CallbackRegister(FLEXCOM_SPI_SLAVE_CALLBACK callBack, uintptr_t context )
{
    flexcom2SpiObj.callback = callBack;

    flexcom2SpiObj.context = context;
}

/* The status is returned busy during the period the chip-select remains asserted */
bool FLEXCOM2_SPI_IsBusy(void)
{
    return flexcom2SpiObj.transferIsBusy;
}

/* Drive the GPIO pin to indicate to SPI Master that the slave is ready now */
void FLEXCOM2_SPI_Ready(void)
{
    PIO_PinWrite((PIO_PIN)PIO_PIN_PD13, 0);
}

FLEXCOM_SPI_SLAVE_ERROR FLEXCOM2_SPI_ErrorGet(void)
{
    FLEXCOM_SPI_SLAVE_ERROR errorStatus = flexcom2SpiObj.errorStatus;

    flexcom2SpiObj.errorStatus = FLEXCOM_SPI_SLAVE_ERROR_NONE;

    return errorStatus;
}

void FLEXCOM2_InterruptHandler(void)
{
    uint8_t txRxData;

    uint32_t statusFlags = FLEXCOM2_REGS->FLEX_SPI_SR;

    if (statusFlags & SPI_SR_OVRES_Msk)
    {
        /*OVRES flag is cleared on reading SPI SR*/

        /* Save the error to report it to application later */
        flexcom2SpiObj.errorStatus = SPI_SR_OVRES_Msk;
    }

    if(statusFlags & SPI_SR_RDRF_Msk)
    {
        flexcom2SpiObj.transferIsBusy = true;

        PIO_PinWrite((PIO_PIN)PIO_PIN_PD13, 1);

        /* Reading DATA register will also clear the RDRF flag */
        txRxData = (FLEXCOM2_REGS->FLEX_SPI_RDR & FLEX_SPI_RDR_RD_Msk) >> FLEX_SPI_RDR_RD_Pos;

        if (flexcom2SpiObj.rdInIndex < FLEXCOM2_READ_BUFFER_SIZE)
        {
            FLEXCOM2_ReadBuffer[flexcom2SpiObj.rdInIndex++] = txRxData;
        }
    }

    if(statusFlags & SPI_SR_TDRE_Msk)
    {
        if (flexcom2SpiObj.wrOutIndex < flexcom2SpiObj.nWrBytes)
        {
            txRxData = FLEXCOM2_WriteBuffer[flexcom2SpiObj.wrOutIndex++];

            FLEXCOM2_REGS->FLEX_SPI_TDR = txRxData;
        }
        else
        {
            /* Disable TDRE interrupt. The last byte sent by the master will be shifted out automatically */
            FLEXCOM2_REGS->FLEX_SPI_IDR = FLEX_SPI_IDR_TDRE_Msk;
        }
    }

    if(statusFlags & SPI_SR_NSSR_Msk)
    {
        /* NSSR flag is cleared on reading SPI SR */

        flexcom2SpiObj.transferIsBusy = false;

        flexcom2SpiObj.wrOutIndex = 0;
        flexcom2SpiObj.nWrBytes = 0;

        if(flexcom2SpiObj.callback != NULL)
        {
            flexcom2SpiObj.callback(flexcom2SpiObj.context);
        }

        /* Clear the rdInIndex. Application must read the received data in the callback. */
        flexcom2SpiObj.rdInIndex = 0;
    }
}