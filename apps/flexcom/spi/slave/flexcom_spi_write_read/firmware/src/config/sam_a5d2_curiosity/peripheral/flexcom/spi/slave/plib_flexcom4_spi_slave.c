/*******************************************************************************
  SPIFLEXCOM4 SPI PLIB

  Company:
    Microchip Technology Inc.

  File Name:
    plib_flexcom4_spi_slave.c

  Summary:
    SPIFLEXCOM4 SPI Slave PLIB Implementation File.

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

#include "plib_flexcom4_spi_slave.h"
#include "peripheral/pio/plib_pio.h"
#include <string.h>
#include "interrupts.h"

#define FLEXCOM_SPI_RDR_REG      (*(volatile uint8_t* const)((FLEXCOM4_BASE_ADDRESS + FLEX_SPI_RDR_REG_OFST)))
#define FLEXCOM_SPI_TDR_REG      (*(volatile uint8_t* const)((FLEXCOM4_BASE_ADDRESS + FLEX_SPI_TDR_REG_OFST)))

#define FLEXCOM4_READ_BUFFER_SIZE            256U
#define FLEXCOM4_WRITE_BUFFER_SIZE           256U

volatile static uint8_t FLEXCOM4_ReadBuffer[FLEXCOM4_READ_BUFFER_SIZE];
volatile static uint8_t FLEXCOM4_WriteBuffer[FLEXCOM4_WRITE_BUFFER_SIZE];

// *****************************************************************************
// *****************************************************************************
// Section: SPIFLEXCOM4 SPI Implementation
// *****************************************************************************
// *****************************************************************************

/* Global object to save FLEXCOM SPI Exchange related data */
volatile static FLEXCOM_SPI_SLAVE_OBJECT flexcom4SpiObj;

static void mem_copy(volatile void* pDst, volatile void* pSrc, uint32_t nBytes)
{
    volatile uint8_t* pSource = (volatile uint8_t*)pSrc;
    volatile uint8_t* pDest = (volatile uint8_t*)pDst;

    for (uint32_t i = 0U; i < nBytes; i++)
    {
        pDest[i] = pSource[i];
    }
}

void FLEXCOM4_SPI_Initialize( void )
{
    /* Set FLEXCOM SPI operating mode */
    FLEXCOM4_REGS->FLEX_MR = FLEX_MR_OPMODE_SPI;

    /* Disable and Reset the SPI*/
    FLEXCOM4_REGS->FLEX_SPI_CR = FLEX_SPI_CR_SPIDIS_Msk | FLEX_SPI_CR_SWRST_Msk;

    /* Enable FIFO support */
    FLEXCOM4_REGS->FLEX_SPI_CR = FLEX_SPI_CR_FIFOEN_Msk;

    /* SPI is by default in Slave Mode, disable mode fault detection */
    FLEXCOM4_REGS->FLEX_SPI_MR = FLEX_SPI_MR_MODFDIS_Msk;

    /* Set up clock Polarity, data phase, Communication Width */
    FLEXCOM4_REGS->FLEX_SPI_CSR[0] = FLEX_SPI_CSR_CPOL(0U) | FLEX_SPI_CSR_NCPHA(1U) | FLEX_SPI_CSR_BITS_8_BIT;

    flexcom4SpiObj.rdInIndex = 0;
    flexcom4SpiObj.wrOutIndex = 0;
    flexcom4SpiObj.nWrBytes = 0;
    flexcom4SpiObj.errorStatus = FLEXCOM_SPI_SLAVE_ERROR_NONE;
    flexcom4SpiObj.callback = NULL ;
    flexcom4SpiObj.transferIsBusy = false ;

    /* Set the Busy Pin to ready state */
    PIO_PinWrite((PIO_PIN)PIO_PIN_PD9, 0);

    /* Enable Receive full and chip deselect interrupt */
    FLEXCOM4_REGS->FLEX_SPI_IER = (FLEX_SPI_IER_RDRF_Msk | FLEX_SPI_IER_NSSR_Msk);

    /* Enable SPIFLEXCOM4 */
    FLEXCOM4_REGS->FLEX_SPI_CR = FLEX_SPI_CR_SPIEN_Msk;
}

/* For 9-bit mode, the "size" must be specified in terms of 16-bit words */
size_t FLEXCOM4_SPI_Read(void* pRdBuffer, size_t size)
{
    size_t rdSize = size;
    size_t rdInIndex = flexcom4SpiObj.rdInIndex;
    uint8_t* pBuffer = (uint8_t*)pRdBuffer;

    if (rdSize > rdInIndex)
    {
        rdSize = rdInIndex;
    }

    (void)mem_copy(pBuffer, FLEXCOM4_ReadBuffer, rdSize);

    return rdSize;
}

/* For 9-bit mode, the "size" must be specified in terms of 16-bit words */
size_t FLEXCOM4_SPI_Write(void* pWrBuffer, size_t size )
{
    uint32_t intState = FLEXCOM4_REGS->FLEX_SPI_IMR;
    size_t wrSize = size;
    uint32_t wrOutIndex = 0;
    uint8_t* pBuffer = (uint8_t*)pWrBuffer;

    FLEXCOM4_REGS->FLEX_SPI_IDR = intState;

    if (wrSize > FLEXCOM4_WRITE_BUFFER_SIZE)
    {
        wrSize = FLEXCOM4_WRITE_BUFFER_SIZE;
    }

    (void)mem_copy(FLEXCOM4_WriteBuffer, pBuffer, wrSize);

    flexcom4SpiObj.nWrBytes = wrSize;

    while (((FLEXCOM4_REGS->FLEX_SPI_SR & FLEX_SPI_SR_TDRE_Msk) != 0U) && (wrOutIndex < wrSize))
    {
        FLEXCOM_SPI_TDR_REG = FLEXCOM4_WriteBuffer[wrOutIndex];
        wrOutIndex++;
    }

    flexcom4SpiObj.wrOutIndex = wrOutIndex;

    /* Restore interrupt enable state and also enable TDRE interrupt */
    FLEXCOM4_REGS->FLEX_SPI_IER = (intState | FLEX_SPI_IER_TDRE_Msk);

    return wrSize;
}

/* For 9-bit mode, the return value is in terms of 16-bit words */
size_t FLEXCOM4_SPI_ReadCountGet(void)
{
    return flexcom4SpiObj.rdInIndex;
}

/* For 9-bit mode, the return value is in terms of 16-bit words */
size_t FLEXCOM4_SPI_ReadBufferSizeGet(void)
{
    return FLEXCOM4_READ_BUFFER_SIZE;
}

/* For 9-bit mode, the return value is in terms of 16-bit words */
size_t FLEXCOM4_SPI_WriteBufferSizeGet(void)
{
    return FLEXCOM4_WRITE_BUFFER_SIZE;
}

void FLEXCOM4_SPI_CallbackRegister(FLEXCOM_SPI_SLAVE_CALLBACK callBack, uintptr_t context )
{
    flexcom4SpiObj.callback = callBack;

    flexcom4SpiObj.context = context;
}

/* The status is returned busy during the period the chip-select remains asserted */
bool FLEXCOM4_SPI_IsBusy(void)
{
    return flexcom4SpiObj.transferIsBusy;
}

/* Drive the GPIO pin to indicate to SPI Master that the slave is ready now */
void FLEXCOM4_SPI_Ready(void)
{
    PIO_PinWrite((PIO_PIN)PIO_PIN_PD9, 0);
}

FLEXCOM_SPI_SLAVE_ERROR FLEXCOM4_SPI_ErrorGet(void)
{
    FLEXCOM_SPI_SLAVE_ERROR errorStatus = flexcom4SpiObj.errorStatus;

    flexcom4SpiObj.errorStatus = FLEXCOM_SPI_SLAVE_ERROR_NONE;

    return errorStatus;
}

void __attribute__((used)) FLEXCOM4_InterruptHandler(void)
{
    uint8_t txRxData = 0;

    uint32_t statusFlags = FLEXCOM4_REGS->FLEX_SPI_SR;

    if ((statusFlags & FLEX_SPI_SR_OVRES_Msk) != 0U)
    {
        /*OVRES flag is cleared on reading SPI SR*/

        /* Save the error to report it to application later */
        flexcom4SpiObj.errorStatus = FLEXCOM_SPI_SLAVE_ERROR_BUFOVF;
    }

    if((statusFlags & FLEX_SPI_SR_RDRF_Msk) != 0U)
    {
        if (flexcom4SpiObj.transferIsBusy == false)
        {
            flexcom4SpiObj.transferIsBusy = true;

            PIO_PinWrite((PIO_PIN)PIO_PIN_PD9, 1);
        }

        uint32_t rdInIndex = flexcom4SpiObj.rdInIndex;

        while (((statusFlags |= FLEXCOM4_REGS->FLEX_SPI_SR) & FLEX_SPI_SR_RDRF_Msk) != 0U)
        {
            /* Reading DATA register will also clear the RDRF flag */
            txRxData = FLEXCOM_SPI_RDR_REG;

            if(rdInIndex < FLEXCOM4_READ_BUFFER_SIZE)
            {
                FLEXCOM4_ReadBuffer[rdInIndex] = txRxData;
                rdInIndex++;
            }

            statusFlags &= ~FLEX_SPI_SR_RDRF_Msk;
        }

        flexcom4SpiObj.rdInIndex = rdInIndex;
    }

    if((statusFlags & FLEX_SPI_SR_TDRE_Msk) != 0U)
    {
        uint32_t wrOutIndex = flexcom4SpiObj.wrOutIndex;
        uint32_t nWrBytes = flexcom4SpiObj.nWrBytes;

        while ((((statusFlags |= FLEXCOM4_REGS->FLEX_SPI_SR) & FLEX_SPI_SR_TDRE_Msk) != 0U) && (wrOutIndex < nWrBytes))
        {
            FLEXCOM_SPI_TDR_REG = FLEXCOM4_WriteBuffer[wrOutIndex];
            wrOutIndex++;
            statusFlags &= ~FLEX_SPI_SR_TDRE_Msk;
        }

        if (wrOutIndex >= nWrBytes)
        {
            /* Disable TDRE interrupt. The last byte sent by the master will be shifted out automatically */
            FLEXCOM4_REGS->FLEX_SPI_IDR = FLEX_SPI_IDR_TDRE_Msk;
        }

        flexcom4SpiObj.wrOutIndex = wrOutIndex;
    }

    if((statusFlags & FLEX_SPI_SR_NSSR_Msk) != 0U)
    {
        /* NSSR flag is cleared on reading SPI SR */

        flexcom4SpiObj.transferIsBusy = false;

        flexcom4SpiObj.wrOutIndex = 0;
        flexcom4SpiObj.nWrBytes = 0;

        if(flexcom4SpiObj.callback != NULL)
        {
            uintptr_t context = flexcom4SpiObj.context;

            flexcom4SpiObj.callback(context);
        }

        /* Clear the rdInIndex. Application must read the received data in the callback. */
        flexcom4SpiObj.rdInIndex = 0;
    }
}