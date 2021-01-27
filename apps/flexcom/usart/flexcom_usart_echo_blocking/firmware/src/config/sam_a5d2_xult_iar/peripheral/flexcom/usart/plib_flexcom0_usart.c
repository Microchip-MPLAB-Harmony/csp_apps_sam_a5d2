/*******************************************************************************
  FLEXCOM0 USART PLIB

  Company:
    Microchip Technology Inc.

  File Name:
    plib_flexcom0_usart.c

  Summary:
    FLEXCOM0 USART PLIB Implementation File

  Description
    This file defines the interface to the FLEXCOM0 USART peripheral library. This
    library provides access to and control of the associated peripheral
    instance.

  Remarks:
    None.
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

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
/* This section lists the other files that are included in this file.
*/
#include "plib_flexcom0_usart.h"
#include "interrupts.h"


// *****************************************************************************
// *****************************************************************************
// Section: FLEXCOM0 USART Implementation
// *****************************************************************************
// *****************************************************************************

void static FLEXCOM0_USART_ErrorClear( void )
{
    uint16_t dummyData = 0;

    if (FLEXCOM0_REGS->FLEX_US_CSR & (FLEX_US_CSR_OVRE_Msk | FLEX_US_CSR_FRAME_Msk | FLEX_US_CSR_PARE_Msk))
    {
        /* Clear the error flags */
        FLEXCOM0_REGS->FLEX_US_CR = FLEX_US_CR_RSTSTA_Msk;

        /* Flush existing error bytes from the RX FIFO */
        while( FLEXCOM0_REGS->FLEX_US_CSR & FLEX_US_CSR_RXRDY_Msk )
        {
            if (FLEXCOM0_REGS->FLEX_US_MR & FLEX_US_MR_MODE9_Msk)
            {
                dummyData = *((uint16_t*)&FLEXCOM0_REGS->FLEX_US_RHR) & FLEX_US_RHR_RXCHR_Msk;
            }
            else
            {
                dummyData = *((uint8_t*)&FLEXCOM0_REGS->FLEX_US_RHR);
            }
        }
    }

    /* Ignore the warning */
    (void)dummyData;

    return;
}


void FLEXCOM0_USART_Initialize( void )
{
    /* Set FLEXCOM USART operating mode */
    FLEXCOM0_REGS->FLEX_MR = FLEX_MR_OPMODE_USART;

    /* Reset FLEXCOM0 USART */
    FLEXCOM0_REGS->FLEX_US_CR = (FLEX_US_CR_RSTRX_Msk | FLEX_US_CR_RSTTX_Msk | FLEX_US_CR_RSTSTA_Msk );


    /* Setup transmitter timeguard register */
    FLEXCOM0_REGS->FLEX_US_TTGR = 0;

    /* Configure FLEXCOM0 USART mode */
    FLEXCOM0_REGS->FLEX_US_MR = ( FLEX_US_MR_USART_MODE_NORMAL | FLEX_US_MR_USCLKS_MCK | FLEX_US_MR_CHRL_8_BIT | FLEX_US_MR_PAR_NO | FLEX_US_MR_NBSTOP_1_BIT | (0 << FLEX_US_MR_OVER_Pos));

    /* Configure FLEXCOM0 USART Baud Rate */
    FLEXCOM0_REGS->FLEX_US_BRGR = FLEX_US_BRGR_CD(45);

    /* Enable FLEXCOM0 USART */
    FLEXCOM0_REGS->FLEX_US_CR = (FLEX_US_CR_TXEN_Msk | FLEX_US_CR_RXEN_Msk);


    return;
}

FLEXCOM_USART_ERROR FLEXCOM0_USART_ErrorGet( void )
{
    FLEXCOM_USART_ERROR errors = FLEXCOM_USART_ERROR_NONE;
    uint32_t status = FLEXCOM0_REGS->FLEX_US_CSR;

    /* Collect all errors */
    if(status & FLEX_US_CSR_OVRE_Msk)
    {
        errors = FLEXCOM_USART_ERROR_OVERRUN;
    }
    if(status & FLEX_US_CSR_PARE_Msk)
    {
        errors |= FLEXCOM_USART_ERROR_PARITY;
    }
    if(status & FLEX_US_CSR_FRAME_Msk)
    {
        errors |= FLEXCOM_USART_ERROR_FRAMING;
    }

    if(errors != FLEXCOM_USART_ERROR_NONE)
    {
        FLEXCOM0_USART_ErrorClear();
    }

    /* All errors are cleared, but send the previous error state */
    return errors;
}

bool FLEXCOM0_USART_SerialSetup( FLEXCOM_USART_SERIAL_SETUP *setup, uint32_t srcClkFreq )
{
    uint32_t baud = 0;
    uint32_t brgVal = 0;
    uint32_t overSampVal = 0;
    uint32_t usartMode;
    bool status = false;

    if (setup != NULL)
    {
        baud = setup->baudRate;
        if(srcClkFreq == 0)
        {
            srcClkFreq = FLEXCOM0_USART_FrequencyGet();
        }

        /* Calculate BRG value */
        if (srcClkFreq >= (16 * baud))
        {
            brgVal = (srcClkFreq / (16 * baud));
        }
        else if (srcClkFreq >= (8 * baud))
        {
            brgVal = (srcClkFreq / (8 * baud));
            overSampVal = (1 << FLEX_US_MR_OVER_Pos) & FLEX_US_MR_OVER_Msk;
        }
        else
        {
            /* The input clock source - srcClkFreq, is too low to generate the desired baud */
            return status;
        }
        if (brgVal > 65535)
        {
            /* The requested baud is so low that the ratio of srcClkFreq to baud exceeds the 16-bit register value of CD register */
            return status;
        }

        /* Configure FLEXCOM0 USART mode */
        usartMode = FLEXCOM0_REGS->FLEX_US_MR;
        usartMode &= ~(FLEX_US_MR_CHRL_Msk | FLEX_US_MR_MODE9_Msk | FLEX_US_MR_PAR_Msk | FLEX_US_MR_NBSTOP_Msk | FLEX_US_MR_OVER_Msk);
        FLEXCOM0_REGS->FLEX_US_MR = usartMode | ((uint32_t)setup->dataWidth | (uint32_t)setup->parity | (uint32_t)setup->stopBits | overSampVal);

        /* Configure FLEXCOM0 USART Baud Rate */
        FLEXCOM0_REGS->FLEX_US_BRGR = FLEX_US_BRGR_CD(brgVal);
        status = true;
    }

    return status;
}

bool FLEXCOM0_USART_Read( void *buffer, const size_t size )
{
    bool status = false;
    uint32_t errorStatus = 0;
    size_t processedSize = 0;
    uint8_t* pBuffer = (uint8_t*)buffer;

    if(pBuffer != NULL)
    {
        /* Clear errors that may have got generated when there was no active read request pending */
        FLEXCOM0_USART_ErrorClear();

        while( processedSize < size )
        {
            while(!(FLEXCOM0_REGS->FLEX_US_CSR & FLEX_US_CSR_RXRDY_Msk));

            /* Read error status */
            errorStatus = (FLEXCOM0_REGS->FLEX_US_CSR & (FLEX_US_CSR_OVRE_Msk | FLEX_US_CSR_FRAME_Msk | FLEX_US_CSR_PARE_Msk));

            if(errorStatus != 0)
            {
                break;
            }

            if (FLEXCOM0_REGS->FLEX_US_MR & FLEX_US_MR_MODE9_Msk)
            {
                *((uint16_t*)pBuffer) = *((uint16_t*)&FLEXCOM0_REGS->FLEX_US_RHR) & FLEX_US_RHR_RXCHR_Msk;
                pBuffer += 2;
            }
            else
            {
                *pBuffer++ = *((uint8_t*)&FLEXCOM0_REGS->FLEX_US_RHR);
            }

            processedSize++;
        }

        if(size == processedSize)
        {
            status = true;
        }
    }

    return status;
}

bool FLEXCOM0_USART_Write( void *buffer, const size_t size )
{
    bool status = false;
    size_t processedSize = 0;
    uint8_t* pBuffer = (uint8_t*)buffer;

    if(pBuffer != NULL)
    {
        while( processedSize < size )
        {
            while (!(FLEXCOM0_REGS->FLEX_US_CSR & FLEX_US_CSR_TXRDY_Msk));

            if (FLEXCOM0_REGS->FLEX_US_MR & FLEX_US_MR_MODE9_Msk)
            {
                *((uint16_t*)&FLEXCOM0_REGS->FLEX_US_THR) = ((uint16_t*)pBuffer)[processedSize++] & FLEX_US_THR_TXCHR_Msk;
            }
            else
            {
                *((uint8_t*)&FLEXCOM0_REGS->FLEX_US_THR) = ((uint8_t*)pBuffer)[processedSize++];
            }
        }

        status = true;
    }

    return status;
}


uint8_t FLEXCOM0_USART_ReadByte(void)
{
    return(FLEXCOM0_REGS->FLEX_US_RHR & FLEX_US_RHR_RXCHR_Msk);
}

void FLEXCOM0_USART_WriteByte(uint8_t data)
{
    while (!(FLEXCOM0_REGS->FLEX_US_CSR & FLEX_US_CSR_TXRDY_Msk));

    FLEXCOM0_REGS->FLEX_US_THR = (FLEX_US_THR_TXCHR(data) & FLEX_US_THR_TXCHR_Msk);
}

bool FLEXCOM0_USART_TransmitterIsReady( void )
{
    if (FLEXCOM0_REGS->FLEX_US_CSR & FLEX_US_CSR_TXRDY_Msk)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool FLEXCOM0_USART_ReceiverIsReady( void )
{
    if (FLEXCOM0_REGS->FLEX_US_CSR & FLEX_US_CSR_RXRDY_Msk)
    {
        return true;
    }
    else
    {
        return false;
    }
}


