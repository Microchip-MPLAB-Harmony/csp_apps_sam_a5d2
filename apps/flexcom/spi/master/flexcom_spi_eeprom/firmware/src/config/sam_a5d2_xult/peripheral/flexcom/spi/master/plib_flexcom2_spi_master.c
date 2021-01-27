/*******************************************************************************
  FLEXCOM2 SPI PLIB

  Company:
    Microchip Technology Inc.

  File Name:
    plib_flexcom2_spi_master.c

  Summary:
    FLEXCOM2 SPI Master PLIB Implementation File.

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

#include "plib_flexcom2_spi_master.h"
#include "interrupts.h"

// *****************************************************************************
// *****************************************************************************
// Section: FLEXCOM2 SPI Implementation
// *****************************************************************************
// *****************************************************************************
/* Global object to save FLEXCOM SPI Exchange related data */
FLEXCOM_SPI_OBJECT flexcom2SpiObj;

void FLEXCOM2_SPI_Initialize ( void )
{
    /* Set FLEXCOM SPI operating mode */
    FLEXCOM2_REGS->FLEX_MR = FLEX_MR_OPMODE_SPI;

    /* Disable and Reset the FLEXCOM SPI */
    FLEXCOM2_REGS->FLEX_SPI_CR = FLEX_SPI_CR_SPIDIS_Msk | FLEX_SPI_CR_SWRST_Msk;

    FLEXCOM2_REGS->FLEX_SPI_CR = FLEX_SPI_CR_FIFOEN_Msk;

    /* Enable Master mode, select clock source, select particular NPCS line for chip select and disable mode fault detection */
    FLEXCOM2_REGS->FLEX_SPI_MR = FLEX_SPI_MR_MSTR_Msk | FLEX_SPI_MR_BRSRCCLK_PERIPH_CLK | FLEX_SPI_MR_DLYBCS(0) | FLEX_SPI_MR_PCS(FLEXCOM_SPI_CHIP_SELECT_NPCS0) | FLEX_SPI_MR_MODFDIS_Msk;

    /* Set up clock Polarity, data phase, Communication Width, Baud Rate */
    FLEXCOM2_REGS->FLEX_SPI_CSR[0]= FLEX_SPI_CSR_CPOL(0) | FLEX_SPI_CSR_NCPHA(1) | FLEX_SPI_CSR_BITS_8_BIT | FLEX_SPI_CSR_SCBR(83) | FLEX_SPI_CSR_DLYBS(0) | FLEX_SPI_CSR_DLYBCT(0) | FLEX_SPI_CSR_CSAAT_Msk;




    /* Initialize global variables */
    flexcom2SpiObj.transferIsBusy = false;
    flexcom2SpiObj.callback = NULL;

    /* Enable FLEXCOM2 SPI */
    FLEXCOM2_REGS->FLEX_SPI_CR = FLEX_SPI_CR_SPIEN_Msk;
    return;
}




static uint8_t FLEXCOM2_SPI_FIFO_Fill(void)
{
    uint8_t nDataCopiedToFIFO = 0;
    uint32_t dataBits = FLEXCOM2_REGS->FLEX_SPI_CSR[0] & FLEX_SPI_CSR_BITS_Msk;

    while ((nDataCopiedToFIFO < 32) && (FLEXCOM2_REGS->FLEX_SPI_SR & FLEX_SPI_SR_TDRE_Msk))
    {
        if(dataBits == FLEX_SPI_CSR_BITS_8_BIT)
        {
            if (flexcom2SpiObj.txCount < flexcom2SpiObj.txSize)
            {
                *((uint8_t*)&FLEXCOM2_REGS->FLEX_SPI_TDR) =  ((uint8_t*)flexcom2SpiObj.txBuffer)[flexcom2SpiObj.txCount++];
            }
            else if (flexcom2SpiObj.dummySize > 0)
            {
                *((uint8_t*)&FLEXCOM2_REGS->FLEX_SPI_TDR) = (uint8_t)(0xff);
                flexcom2SpiObj.dummySize--;
            }
            else
            {
                break;
            }
        }
        else
        {
            if (flexcom2SpiObj.txCount < flexcom2SpiObj.txSize)
            {
                *((uint16_t*)&FLEXCOM2_REGS->FLEX_SPI_TDR) =  ((uint16_t*)flexcom2SpiObj.txBuffer)[flexcom2SpiObj.txCount++];
            }
            else if (flexcom2SpiObj.dummySize > 0)
            {
                *((uint16_t*)&FLEXCOM2_REGS->FLEX_SPI_TDR) = (uint16_t)(0xffff);
                flexcom2SpiObj.dummySize--;
            }
            else
            {
                break;
            }
        }

        nDataCopiedToFIFO++;
    }

    return nDataCopiedToFIFO;
}

bool FLEXCOM2_SPI_WriteRead (void* pTransmitData, size_t txSize, void* pReceiveData, size_t rxSize)
{
    bool isRequestAccepted = false;
    uint32_t nTxPending = 0;
    uint8_t rxThreshold = 0;

    /* Verify the request */
    if((((txSize > 0) && (pTransmitData != NULL)) || ((rxSize > 0) && (pReceiveData != NULL))) && (flexcom2SpiObj.transferIsBusy == false))
    {
        isRequestAccepted = true;
        flexcom2SpiObj.txBuffer = pTransmitData;
        flexcom2SpiObj.rxBuffer = pReceiveData;
        flexcom2SpiObj.rxCount = 0;
        flexcom2SpiObj.txCount = 0;
        flexcom2SpiObj.dummySize = 0;

        if (pTransmitData != NULL)
        {
            flexcom2SpiObj.txSize = txSize;
        }
        else
        {
            flexcom2SpiObj.txSize = 0;
        }

        if (pReceiveData != NULL)
        {
            flexcom2SpiObj.rxSize = rxSize;
        }
        else
        {
            flexcom2SpiObj.rxSize = 0;
        }

        flexcom2SpiObj.transferIsBusy = true;

        if (flexcom2SpiObj.rxSize > flexcom2SpiObj.txSize)
        {
            flexcom2SpiObj.dummySize = flexcom2SpiObj.rxSize - flexcom2SpiObj.txSize;
        }

        if((FLEXCOM2_REGS->FLEX_SPI_CSR[0] & FLEX_SPI_CSR_BITS_Msk) != FLEX_SPI_CSR_BITS_8_BIT)
        {
            flexcom2SpiObj.txSize >>= 1;
            flexcom2SpiObj.dummySize >>= 1;
            flexcom2SpiObj.rxSize >>= 1;
        }

        /* Clear TX and RX FIFO */
        FLEXCOM2_REGS->FLEX_SPI_CR = (FLEX_SPI_CR_RXFCLR_Msk | FLEX_SPI_CR_TXFCLR_Msk);

        nTxPending = (flexcom2SpiObj.txSize - flexcom2SpiObj.txCount) + flexcom2SpiObj.dummySize;

        if (nTxPending < 32)
        {
            rxThreshold = nTxPending;
        }
        else
        {
            rxThreshold = 32;
        }

        /* Set RX FIFO level so as to generate interrupt after all bytes are transmitted and response from slave is received for all the bytes */
        /* RX FIFO level must be set first or else FIFO may be filled before RX threshold is set and hardware may not recognize threshold crossover and not generate threshold interrupt */
        FLEXCOM2_REGS->FLEX_SPI_FMR = (FLEXCOM2_REGS->FLEX_SPI_FMR & ~FLEX_SPI_FMR_RXFTHRES_Msk) | FLEX_SPI_FMR_RXFTHRES(rxThreshold);

        (void) FLEXCOM2_SPI_FIFO_Fill();

        /* Enable RX FIFO Threshold interrupt */
        FLEXCOM2_REGS->FLEX_SPI_IER = FLEX_SPI_IER_RXFTHF_Msk;
    }

    return isRequestAccepted;
}

bool FLEXCOM2_SPI_TransferSetup (FLEXCOM_SPI_TRANSFER_SETUP * setup, uint32_t spiSourceClock )
{
    uint32_t scbr;
    if ((setup == NULL) || (setup->clockFrequency == 0))
    {
        return false;
    }
    if(spiSourceClock == 0)
    {
        // Fetch Master Clock Frequency directly
        spiSourceClock = 83000000;
    }

    scbr = spiSourceClock/setup->clockFrequency;

    if(scbr == 0)
    {
        scbr = 1;
    }
    else if(scbr > 255)
    {
        scbr = 255;
    }

    FLEXCOM2_REGS->FLEX_SPI_CSR[0]= (FLEXCOM2_REGS->FLEX_SPI_CSR[0] & ~(FLEX_SPI_CSR_CPOL_Msk | FLEX_SPI_CSR_NCPHA_Msk | FLEX_SPI_CSR_BITS_Msk | FLEX_SPI_CSR_SCBR_Msk)) | ((uint32_t)setup->clockPolarity | (uint32_t)setup->clockPhase | (uint32_t)setup->dataBits | FLEX_SPI_CSR_SCBR(scbr));

    return true;
}

bool FLEXCOM2_SPI_Write(void* pTransmitData, size_t txSize)
{
    return(FLEXCOM2_SPI_WriteRead(pTransmitData, txSize, NULL, 0));
}

bool FLEXCOM2_SPI_Read(void* pReceiveData, size_t rxSize)
{
    return(FLEXCOM2_SPI_WriteRead(NULL, 0, pReceiveData, rxSize));
}

void FLEXCOM2_SPI_CallbackRegister (FLEXCOM_SPI_CALLBACK callback, uintptr_t context)
{
    flexcom2SpiObj.callback = callback;
    flexcom2SpiObj.context = context;
}

bool FLEXCOM2_SPI_IsBusy(void)
{
    return ((flexcom2SpiObj.transferIsBusy) || ((FLEXCOM2_REGS->FLEX_SPI_SR & FLEX_SPI_SR_TXEMPTY_Msk) == 0));
}

void FLEXCOM2_InterruptHandler(void)
{
    uint32_t dataBits = FLEXCOM2_REGS->FLEX_SPI_CSR[0] & FLEX_SPI_CSR_BITS_Msk;
    uint32_t nTxPending = 0;
    uint8_t rxThreshold = 0;

    while ((FLEXCOM2_REGS->FLEX_SPI_SR & FLEX_SPI_SR_RDRF_Msk ) && (flexcom2SpiObj.rxCount < flexcom2SpiObj.rxSize))
    {
        if(dataBits == FLEX_SPI_CSR_BITS_8_BIT)
        {
            ((uint8_t*)flexcom2SpiObj.rxBuffer)[flexcom2SpiObj.rxCount++] = *((uint8_t*)&FLEXCOM2_REGS->FLEX_SPI_RDR);
        }
        else
        {
            ((uint16_t*)flexcom2SpiObj.rxBuffer)[flexcom2SpiObj.rxCount++] = *((uint16_t*)&FLEXCOM2_REGS->FLEX_SPI_RDR);
        }
    }

    /* Clear RX FIFO. This is done for the case where RX size is less than TX size and hence data is not read and copied into the application rx buffer. */
    FLEXCOM2_REGS->FLEX_SPI_CR = FLEX_SPI_CR_RXFCLR_Msk;

    nTxPending = (flexcom2SpiObj.txSize - flexcom2SpiObj.txCount) + flexcom2SpiObj.dummySize;

    if (nTxPending > 0)
    {
        if (nTxPending < 32)
        {
            rxThreshold = nTxPending;
        }
        else
        {
            rxThreshold = 32;
        }

        /* Set RX FIFO level so as to generate interrupt after all bytes are transmitted and response from slave is received for all the bytes */
        /* RX FIFO level must be set first or else FIFO may be filled before RX threshold is set and hardware may not recognize threshold crossover and not generate threshold interrupt */
        FLEXCOM2_REGS->FLEX_SPI_FMR = (FLEXCOM2_REGS->FLEX_SPI_FMR & ~FLEX_SPI_FMR_RXFTHRES_Msk) | FLEX_SPI_FMR_RXFTHRES(rxThreshold);

        (void) FLEXCOM2_SPI_FIFO_Fill();
    }
    else
    {
        /* Set Last transfer to deassert NPCS after the last byte written in TDR has been transferred. */
        FLEXCOM2_REGS->FLEX_SPI_CR = FLEX_SPI_CR_LASTXFER_Msk;

        flexcom2SpiObj.transferIsBusy = false;

        /* Disable Receive FIFO Threshold interrupt */
        FLEXCOM2_REGS->FLEX_SPI_IDR = FLEX_SPI_IDR_RXFTHF_Msk;

        if(flexcom2SpiObj.callback != NULL)
        {
            flexcom2SpiObj.callback(flexcom2SpiObj.context);
        }
    }
}


/*******************************************************************************
 End of File
*/

