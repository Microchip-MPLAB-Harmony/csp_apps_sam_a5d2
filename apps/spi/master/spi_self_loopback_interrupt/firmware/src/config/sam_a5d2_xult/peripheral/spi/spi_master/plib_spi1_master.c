/*******************************************************************************
  SPI PLIB

  Company:
    Microchip Technology Inc.

  File Name:
    plib_spi1_master.c

  Summary:
    SPI1 Master Source File

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

#include "plib_spi1_master.h"
#include "interrupts.h"

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
volatile static SPI_OBJECT spi1Obj;

void SPI1_Initialize( void )
{
    /* Disable and Reset the SPI*/
    SPI1_REGS->SPI_CR = SPI_CR_SPIDIS_Msk | SPI_CR_SWRST_Msk;


    /* Enable Master mode, select source clock, select particular NPCS line for chip select and disable mode fault detection */
    SPI1_REGS->SPI_MR = SPI_MR_MSTR_Msk | SPI_MR_DLYBCS(0) | SPI_MR_BRSRCCLK_PERIPH_CLK | SPI_MR_PCS_NPCS0  | SPI_MR_MODFDIS_Msk;

    /* Set up clock Polarity, data phase, Communication Width, Baud Rate */
    SPI1_REGS->SPI_CSR[0] = SPI_CSR_CPOL_IDLE_LOW | SPI_CSR_NCPHA_VALID_LEADING_EDGE | SPI_CSR_BITS_8_BIT | SPI_CSR_SCBR(83)| SPI_CSR_DLYBS(0) | SPI_CSR_DLYBCT(0)  | SPI_CSR_CSAAT(1) ;





    /* Initialize global variables */
    spi1Obj.transferIsBusy = false;
    spi1Obj.callback = NULL;

    /* Enable SPI1 */
    SPI1_REGS->SPI_CR = SPI_CR_SPIEN_Msk;
}



bool SPI1_WriteRead( void* pTransmitData, size_t txSize, void* pReceiveData, size_t rxSize )
{
    bool isRequestAccepted = false;
    uint32_t dummyData;

    /* Verify the request */
    if((spi1Obj.transferIsBusy == false) && (((txSize > 0U) && (pTransmitData != NULL)) || ((rxSize > 0U) && (pReceiveData != NULL))))
    {
        isRequestAccepted = true;
        spi1Obj.txBuffer = pTransmitData;
        spi1Obj.rxBuffer = pReceiveData;
        spi1Obj.rxCount = 0;
        spi1Obj.txCount = 0;
        spi1Obj.dummySize = 0;

        if (pTransmitData != NULL)
        {
            spi1Obj.txSize = txSize;
        }
        else
        {
            spi1Obj.txSize = 0;
        }

        if (pReceiveData != NULL)
        {
            spi1Obj.rxSize = rxSize;
        }
        else
        {
            spi1Obj.rxSize = 0;
        }

        spi1Obj.transferIsBusy = true;

        /* Flush out any unread data in SPI read buffer */
        dummyData = (SPI1_REGS->SPI_RDR & SPI_RDR_RD_Msk) >> SPI_RDR_RD_Pos;
        (void)dummyData;

        size_t txSz = spi1Obj.txSize;

        if (spi1Obj.rxSize > txSz)
        {
            spi1Obj.dummySize = spi1Obj.rxSize - txSz;
        }

        /* Start the first write here itself, rest will happen in ISR context */
        if((SPI1_REGS->SPI_CSR[0] & SPI_CSR_BITS_Msk) == SPI_CSR_BITS_8_BIT)
        {
            if (spi1Obj.txCount < txSz)
            {
                SPI1_REGS->SPI_TDR = *((uint8_t*)spi1Obj.txBuffer);
                spi1Obj.txCount++;
            }
            else if (spi1Obj.dummySize > 0U)
            {
                SPI1_REGS->SPI_TDR = (uint8_t)(0xff);
                spi1Obj.dummySize--;
            }
            else
            {
                /* Do Nothing */
            }
        }
        else
        {
            spi1Obj.txSize >>= 1;
            spi1Obj.dummySize >>= 1;
            spi1Obj.rxSize >>= 1;

            txSz = spi1Obj.txSize;

            if (spi1Obj.txCount < txSz)
            {
                SPI1_REGS->SPI_TDR = *((uint16_t*)spi1Obj.txBuffer);
                spi1Obj.txCount++;
            }
            else if (spi1Obj.dummySize > 0U)
            {
                SPI1_REGS->SPI_TDR = (uint16_t)(0xffff);
                spi1Obj.dummySize--;
            }
            else
            {
                /* Do Nothing */

            }
        }

        if (rxSize > 0U)
        {
            /* Enable receive interrupt to complete the transfer in ISR context */
            SPI1_REGS->SPI_IER = SPI_IER_RDRF_Msk;
        }
        else
        {
            /* Enable transmit interrupt to complete the transfer in ISR context */
            SPI1_REGS->SPI_IER = SPI_IER_TDRE_Msk;
        }
    }

    return isRequestAccepted;
}


bool SPI1_Write( void* pTransmitData, size_t txSize )
{
    return(SPI1_WriteRead(pTransmitData, txSize, NULL, 0));
}

bool SPI1_Read( void* pReceiveData, size_t rxSize )
{
    return(SPI1_WriteRead(NULL, 0, pReceiveData, rxSize));
}

bool SPI1_TransferSetup( SPI_TRANSFER_SETUP * setup, uint32_t spiSourceClock )
{
    uint32_t scbr;

    if ((setup == NULL) || (setup->clockFrequency == 0U))
    {
        return false;
    }

    if(spiSourceClock == 0U)
    {
        // Fetch Master Clock Frequency directly
        spiSourceClock = 83000000;
    }

    scbr = spiSourceClock/setup->clockFrequency;

    if(scbr == 0U)
    {
        scbr = 1U;
    }
    else if(scbr > 255U)
    {
        scbr = 255U;
    }
    else
    {
        /* Do Nothing */
    }

    SPI1_REGS->SPI_CSR[0] = (SPI1_REGS->SPI_CSR[0] & ~(SPI_CSR_CPOL_Msk | SPI_CSR_NCPHA_Msk | SPI_CSR_BITS_Msk | SPI_CSR_SCBR_Msk)) |((uint32_t)setup->clockPolarity | (uint32_t)setup->clockPhase | (uint32_t)setup->dataBits | SPI_CSR_SCBR(scbr));

    return true;
}

bool SPI1_IsTransmitterBusy( void )
{
    return ((SPI1_REGS->SPI_SR & SPI_SR_TXEMPTY_Msk) == 0U)? true : false;
}

void SPI1_CallbackRegister( SPI_CALLBACK callback, uintptr_t context )
{
    spi1Obj.callback = callback;
    spi1Obj.context = context;
}

bool SPI1_IsBusy( void )
{
    bool transferIsBusy = spi1Obj.transferIsBusy;

    return (((SPI1_REGS->SPI_SR & SPI_SR_TXEMPTY_Msk) == 0U) || (transferIsBusy));
}
void __attribute__((used)) SPI1_InterruptHandler( void )
{
    uint32_t dataBits;
    uint32_t receivedData;
    static bool isLastByteTransferInProgress = false;

    dataBits = SPI1_REGS->SPI_CSR[0] & SPI_CSR_BITS_Msk;


    size_t rxCount = spi1Obj.rxCount;

    if ((SPI1_REGS->SPI_SR & SPI_SR_RDRF_Msk ) == SPI_SR_RDRF_Msk)
    {
        receivedData = (SPI1_REGS->SPI_RDR & SPI_RDR_RD_Msk) >> SPI_RDR_RD_Pos;

        if (rxCount < spi1Obj.rxSize)
        {
            if(dataBits == SPI_CSR_BITS_8_BIT)
            {
                ((uint8_t*)spi1Obj.rxBuffer)[rxCount] =(uint8_t)receivedData;
                rxCount++;
            }
            else
            {
                ((uint16_t*)spi1Obj.rxBuffer)[rxCount] = (uint16_t)receivedData;
                 rxCount++;
            }
        }

        spi1Obj.rxCount = rxCount;
    }

    /* If there are more words to be transmitted, then transmit them here and keep track of the count */
    if((SPI1_REGS->SPI_SR & SPI_SR_TDRE_Msk) == SPI_SR_TDRE_Msk)
    {
        /* Disable the TDRE interrupt. This will be enabled back if more than
         * one byte is pending to be transmitted */
        SPI1_REGS->SPI_IDR = SPI_IDR_TDRE_Msk;

        size_t txCount = spi1Obj.txCount;

        if(dataBits == SPI_CSR_BITS_8_BIT)
        {
            if (txCount < spi1Obj.txSize)
            {
                SPI1_REGS->SPI_TDR = ((uint8_t*)spi1Obj.txBuffer)[txCount];
                txCount++;
            }
            else if (spi1Obj.dummySize > 0U)
            {
                SPI1_REGS->SPI_TDR = (uint8_t)(0xff);
                spi1Obj.dummySize--;
            }
            else
            {
                /* Do Nothing */
            }
        }
        else
        {
            if (txCount < spi1Obj.txSize)
            {
                SPI1_REGS->SPI_TDR = ((uint16_t*)spi1Obj.txBuffer)[txCount];
                txCount++;
            }
            else if (spi1Obj.dummySize > 0U)
            {
                SPI1_REGS->SPI_TDR = (uint16_t)(0xffff);
                spi1Obj.dummySize--;
            }
            else
            {
                /* Do Nothing */
            }
        }
        size_t txSize = spi1Obj.txSize;

        if ((spi1Obj.dummySize == 0U) && (txCount == txSize))
        {
            /* At higher baud rates, the data in the shift register can be
             * shifted out and TXEMPTY flag can get set resulting in a
             * callback been given to the application with the SPI interrupt
             * pending with the application. This will then result in the
             * interrupt handler being called again with nothing to transmit.
             * To avoid this, a software flag is set, but
             * the TXEMPTY interrupt is not enabled until the very end.
             */

            isLastByteTransferInProgress = true;

        }
        else if (rxCount == spi1Obj.rxSize)
        {
            /* Enable TDRE interrupt as all the requested bytes are received
             * and can now make use of the internal transmit shift register.
             */
            SPI1_REGS->SPI_IDR = SPI_IDR_RDRF_Msk;
            SPI1_REGS->SPI_IER = SPI_IDR_TDRE_Msk;
        }
        else
        {
            /* Do Nothing */
        }

        spi1Obj.txCount = txCount;
    }

    /* See if Exchange is complete */
    if ((isLastByteTransferInProgress == true) && ((SPI1_REGS->SPI_SR & SPI_SR_TXEMPTY_Msk) == SPI_SR_TXEMPTY_Msk))
    {
        if (rxCount == spi1Obj.rxSize)
        {
            /* Set Last transfer to deassert NPCS after the last byte written in TDR has been transferred. */
            SPI1_REGS->SPI_CR = SPI_CR_LASTXFER_Msk;

            spi1Obj.transferIsBusy = false;

            /* Disable TDRE, RDRF and TXEMPTY interrupts */
            SPI1_REGS->SPI_IDR = SPI_IDR_TDRE_Msk | SPI_IDR_RDRF_Msk | SPI_IDR_TXEMPTY_Msk;

            isLastByteTransferInProgress = false;

            if(spi1Obj.callback != NULL)
            {
                uintptr_t context = spi1Obj.context;

                spi1Obj.callback(context);
            }
        }
    }
    if (isLastByteTransferInProgress == true)
    {
        /* For the last byte transfer, the TDRE interrupt is already disabled.
         * Enable TXEMPTY interrupt to ensure no data is present in the shift
         * register before application callback is called.
         */
        SPI1_REGS->SPI_IER = SPI_IER_TXEMPTY_Msk;
    }
}
