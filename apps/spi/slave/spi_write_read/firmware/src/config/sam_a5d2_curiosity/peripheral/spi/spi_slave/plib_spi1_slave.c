/*******************************************************************************
  SPI PLIB

  Company:
    Microchip Technology Inc.

  File Name:
    plib_spi1_slave.c

  Summary:
    SPI1 Slave Source File

  Description:
    This file has implementation of all the interfaces provided for particular
    SPI peripheral.

*******************************************************************************/

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

#include "plib_spi1_slave.h"
#include "peripheral/pio/plib_pio.h"
#include "interrupts.h"
#include <string.h>

#define SPI1_READ_BUFFER_SIZE            256U
#define SPI1_WRITE_BUFFER_SIZE           256U

volatile static uint8_t SPI1_ReadBuffer[SPI1_READ_BUFFER_SIZE];
volatile static uint8_t SPI1_WriteBuffer[SPI1_WRITE_BUFFER_SIZE];


#define NOP asm("nop")


#define SPI_TDR_8BIT_REG      (*(volatile uint8_t* const)((SPI1_BASE_ADDRESS + SPI_TDR_REG_OFST)))

#define SPI_TDR_9BIT_REG      (*(volatile uint16_t* const)((SPI1_BASE_ADDRESS + SPI_TDR_REG_OFST)))



#define SPI_RDR_8BIT_REG      (*(volatile uint8_t* const)((SPI1_BASE_ADDRESS + SPI_RDR_REG_OFST)))

#define SPI_RDR_9BIT_REG      (*(volatile uint16_t* const)((SPI1_BASE_ADDRESS + SPI_RDR_REG_OFST)))
// *****************************************************************************
// *****************************************************************************
// Section: SPI1 Implementation
// *****************************************************************************
// *****************************************************************************

/* Global object to save SPI Exchange related data */
volatile static SPI_SLAVE_OBJECT spi1Obj;

static void mem_copy(volatile void* pDst, volatile void* pSrc, uint32_t nBytes)
{
    volatile uint8_t* pSource = (volatile uint8_t*)pSrc;
    volatile uint8_t* pDest = (volatile uint8_t*)pDst;

    for (uint32_t i = 0U; i < nBytes; i++)
    {
        pDest[i] = pSource[i];
    }
}

void SPI1_Initialize( void )
{
    /* Disable and Reset the SPI*/
    SPI1_REGS->SPI_CR = SPI_CR_SPIDIS_Msk | SPI_CR_SWRST_Msk;

    /* Enable FIFO support */
    SPI1_REGS->SPI_CR = SPI_CR_FIFOEN_Msk;

    /* SPI is by default in Slave Mode, disable mode fault detection */
    SPI1_REGS->SPI_MR = SPI_MR_MODFDIS_Msk;

    /* Set up clock Polarity, data phase, Communication Width */
    SPI1_REGS->SPI_CSR[0] = SPI_CSR_CPOL_IDLE_LOW | SPI_CSR_NCPHA_VALID_LEADING_EDGE | SPI_CSR_BITS_8_BIT;

    spi1Obj.rdInIndex = 0;
    spi1Obj.wrOutIndex = 0;
    spi1Obj.nWrBytes = 0;
    spi1Obj.errorStatus = SPI_SLAVE_ERROR_NONE;
    spi1Obj.callback = NULL ;
    spi1Obj.transferIsBusy = false ;

    /* Set the Busy Pin to ready state */
    PIO_PinWrite((PIO_PIN)PIO_PIN_PD25, 0);

    /* Enable Receive full and chip deselect interrupt */
    SPI1_REGS->SPI_IER = (SPI_IER_RDRF_Msk | SPI_IER_NSSR_Msk);

    /* Enable SPI1 */
    SPI1_REGS->SPI_CR = SPI_CR_SPIEN_Msk;
}

/* For 9-bit mode, the "size" must be specified in terms of 16-bit words */
size_t SPI1_Read(void* pRdBuffer, size_t size)
{
    size_t rdSize = size;
    size_t rdInIndex = spi1Obj.rdInIndex;

    if (rdSize > rdInIndex)
    {
        rdSize = rdInIndex;
    }
    (void) mem_copy(pRdBuffer, SPI1_ReadBuffer, rdSize);

    return rdSize;
}

/* For 9-bit mode, the "size" must be specified in terms of 16-bit words */
size_t SPI1_Write(void* pWrBuffer, size_t size )
{
    uint32_t intState = SPI1_REGS->SPI_IMR;
    size_t wrSize = size;
    uint32_t wrOutIndex = 0;

    SPI1_REGS->SPI_IDR = intState;

    if (wrSize > SPI1_WRITE_BUFFER_SIZE)
    {
        wrSize = SPI1_WRITE_BUFFER_SIZE;
    }

    (void) mem_copy(SPI1_WriteBuffer, pWrBuffer, wrSize);

    spi1Obj.nWrBytes = wrSize;

    while (((SPI1_REGS->SPI_SR & SPI_SR_TDRE_Msk) != 0U) && (wrOutIndex < wrSize))
    {
        SPI_TDR_8BIT_REG = SPI1_WriteBuffer[wrOutIndex];
        wrOutIndex++;
        NOP;
    }

    spi1Obj.wrOutIndex = wrOutIndex;

    /* Restore interrupt enable state and also enable TDRE interrupt */
    SPI1_REGS->SPI_IER = (intState | SPI_IER_TDRE_Msk);

    return wrSize;
}

/* For 9-bit mode, the return value is in terms of 16-bit words */
size_t SPI1_ReadCountGet(void)
{
    return spi1Obj.rdInIndex;
}

/* For 9-bit mode, the return value is in terms of 16-bit words */
size_t SPI1_ReadBufferSizeGet(void)
{
    return SPI1_READ_BUFFER_SIZE;
}

/* For 9-bit mode, the return value is in terms of 16-bit words */
size_t SPI1_WriteBufferSizeGet(void)
{
    return SPI1_WRITE_BUFFER_SIZE;
}

void SPI1_CallbackRegister(SPI_SLAVE_CALLBACK callBack, uintptr_t context )
{
    spi1Obj.callback = callBack;

    spi1Obj.context = context;
}

/* The status is returned busy during the period the chip-select remains asserted */
bool SPI1_IsBusy(void)
{
    return spi1Obj.transferIsBusy;
}

/* Drive the GPIO pin to indicate to SPI Master that the slave is ready now */
void SPI1_Ready(void)
{
    PIO_PinWrite((PIO_PIN)PIO_PIN_PD25, 0);
}

SPI_SLAVE_ERROR SPI1_ErrorGet(void)
{
    SPI_SLAVE_ERROR errorStatus = spi1Obj.errorStatus;

    spi1Obj.errorStatus = SPI_SLAVE_ERROR_NONE;

    return errorStatus;
}

void __attribute__((used)) SPI1_InterruptHandler(void)
{
    uint8_t txRxData = 0;

    volatile uint32_t statusFlags = SPI1_REGS->SPI_SR;

    if ((statusFlags & SPI_SR_OVRES_Msk)  != 0U)
    {
        /*OVRES flag is cleared on reading SPI SR*/

        /* Save the error to report it to application later */
        spi1Obj.errorStatus = SPI_SR_OVRES_Msk;
    }

    if((statusFlags & SPI_SR_RDRF_Msk) != 0U)
    {
        if (spi1Obj.transferIsBusy == false)
        {
            spi1Obj.transferIsBusy = true;

            PIO_PinWrite((PIO_PIN)PIO_PIN_PD25, 1);
        }

        /* Note: statusFlags must be updated every time SPI_SR is read. This is because the NSSR flag
         * is cleared on SPI_SR read. If statusFlags is not updated, there is a possibility of missing
         * NSSR event flag.
         */
        uint32_t rdInIndex = spi1Obj.rdInIndex;

        while (((statusFlags |= SPI1_REGS->SPI_SR)  & SPI_SR_RDRF_Msk) != 0U)
        {
            /* Reading DATA register will also clear the RDRF flag */
            txRxData = SPI_RDR_8BIT_REG;

            if (rdInIndex < SPI1_READ_BUFFER_SIZE)
            {
                SPI1_ReadBuffer[rdInIndex] = txRxData;
                rdInIndex++;
            }

            /* Only clear RDRF flag so as not to clear NSSR flag which may have been set */
            statusFlags &= ~SPI_SR_RDRF_Msk;
        }

        spi1Obj.rdInIndex = rdInIndex;
    }

    if((statusFlags & SPI_SR_TDRE_Msk) != 0U)
    {
        uint32_t wrOutIndex = spi1Obj.wrOutIndex;
        uint32_t nWrBytes = spi1Obj.nWrBytes;

        while ((((statusFlags |= SPI1_REGS->SPI_SR) & SPI_SR_TDRE_Msk) != 0U) && (wrOutIndex < nWrBytes))
        {
            SPI_TDR_8BIT_REG = SPI1_WriteBuffer[wrOutIndex];
            wrOutIndex++;
            /* Only clear TDRE flag so as not to clear NSSR flag which may have been set */
            statusFlags &= ~SPI_SR_TDRE_Msk;
        }

        spi1Obj.wrOutIndex = wrOutIndex;

        if (wrOutIndex >= spi1Obj.nWrBytes)
        {
            /* Disable TDRE interrupt. The last byte sent by the master will be shifted out automatically */
            SPI1_REGS->SPI_IDR = SPI_IDR_TDRE_Msk;
        }
    }

    if((statusFlags & SPI_SR_NSSR_Msk) != 0U)
    {
        /* NSSR flag is cleared on reading SPI SR */

        spi1Obj.transferIsBusy = false;

        spi1Obj.wrOutIndex = 0;
        spi1Obj.nWrBytes = 0;

        if(spi1Obj.callback != NULL)
        {
            uintptr_t context = spi1Obj.context;

            spi1Obj.callback(context);
        }

        /* Clear the rdInIndex. Application must read the received data in the callback. */
        spi1Obj.rdInIndex = 0;
    }
}