/*******************************************************************************
  FLEXCOM4 USART PLIB

  Company:
    Microchip Technology Inc.

  File Name:
    plib_flexcom4_usart.c

  Summary:
    FLEXCOM4 USART PLIB Implementation File

  Description
    This file defines the interface to the FLEXCOM4 USART
    peripheral library. This library provides access to and control of the
    associated peripheral instance.

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

#include "plib_flexcom4_usart.h"
#include "interrupts.h"

#define FLEXCOM_USART_RHR_8BIT_REG      (*(volatile uint8_t* const)((FLEXCOM4_BASE_ADDRESS + FLEX_US_RHR_REG_OFST)))
#define FLEXCOM_USART_RHR_9BIT_REG      (*(volatile uint16_t* const)((FLEXCOM4_BASE_ADDRESS + FLEX_US_RHR_REG_OFST)))

#define FLEXCOM_USART_THR_8BIT_REG      (*(volatile uint8_t* const)((FLEXCOM4_BASE_ADDRESS + FLEX_US_THR_REG_OFST)))
#define FLEXCOM_USART_THR_9BIT_REG      (*(volatile uint16_t* const)((FLEXCOM4_BASE_ADDRESS + FLEX_US_THR_REG_OFST)))

#ifndef FLEX_US_IER_LIN_FRAME_Msk
#define FLEX_US_IER_LIN_FRAME_Msk   FLEX_US_IER_FRAME_Msk
#endif
#ifndef FLEX_US_IER_LIN_PARE_Msk
#define FLEX_US_IER_LIN_PARE_Msk    FLEX_US_IER_PARE_Msk
#endif
#ifndef FLEX_US_IER_LIN_TIMEOUT_Msk
#define FLEX_US_IER_LIN_TIMEOUT_Msk FLEX_US_IER_TIMEOUT_Msk
#endif
#ifndef FLEX_US_IDR_LIN_FRAME_Msk
#define FLEX_US_IDR_LIN_FRAME_Msk   FLEX_US_IDR_FRAME_Msk
#endif
#ifndef FLEX_US_IDR_LIN_PARE_Msk
#define FLEX_US_IDR_LIN_PARE_Msk    FLEX_US_IDR_PARE_Msk
#endif
#ifndef FLEX_US_IDR_LIN_TIMEOUT_Msk
#define FLEX_US_IDR_LIN_TIMEOUT_Msk FLEX_US_IDR_TIMEOUT_Msk
#endif
#ifndef FLEX_US_CSR_LIN_FRAME_Msk
#define FLEX_US_CSR_LIN_FRAME_Msk   FLEX_US_CSR_FRAME_Msk
#endif
#ifndef FLEX_US_CSR_LIN_PARE_Msk
#define FLEX_US_CSR_LIN_PARE_Msk    FLEX_US_CSR_PARE_Msk
#endif


/* Disable Read, Overrun, Parity and Framing error interrupts */
#define FLEXCOM4_USART_RX_INT_DISABLE()      FLEXCOM4_REGS->FLEX_US_IDR = (FLEX_US_IDR_RXRDY_Msk |  FLEX_US_IDR_OVRE_Msk | FLEX_US_IDR_LIN_FRAME_Msk | FLEX_US_IDR_LIN_PARE_Msk | FLEX_US_IDR_LIN_TIMEOUT_Msk \
                                                            | FLEX_US_IDR_LIN_LINBE_Msk | FLEX_US_IDR_LIN_LINISFE_Msk | FLEX_US_IDR_LIN_LINIPE_Msk \
                                                            | FLEX_US_IDR_LIN_LINCE_Msk | FLEX_US_IDR_LIN_LINSNRE_Msk | FLEX_US_IDR_LIN_LINSTE_Msk | FLEX_US_IDR_LIN_LINHTE_Msk)

/* Enable Read, Overrun, Parity and Framing error interrupts */
#define FLEXCOM4_USART_RX_INT_ENABLE()       FLEXCOM4_REGS->FLEX_US_IER = (FLEX_US_IER_RXRDY_Msk | FLEX_US_IER_OVRE_Msk | FLEX_US_IER_LIN_FRAME_Msk | FLEX_US_IER_LIN_PARE_Msk | FLEX_US_IER_LIN_TIMEOUT_Msk \
                                                            | FLEX_US_IER_LIN_LINBE_Msk | FLEX_US_IER_LIN_LINISFE_Msk | FLEX_US_IER_LIN_LINIPE_Msk \
                                                            | FLEX_US_IER_LIN_LINCE_Msk | FLEX_US_IER_LIN_LINSNRE_Msk | FLEX_US_IER_LIN_LINSTE_Msk | FLEX_US_IER_LIN_LINHTE_Msk)

#define FLEXCOM4_USART_TX_INT_DISABLE()      FLEXCOM4_REGS->FLEX_US_IDR = FLEX_US_IDR_TXRDY_Msk
#define FLEXCOM4_USART_TX_INT_ENABLE()       FLEXCOM4_REGS->FLEX_US_IER = FLEX_US_IER_TXRDY_Msk

#define FLEXCOM4_USART_READ_BUFFER_SIZE             128U
#define FLEXCOM4_USART_9BIT_READ_BUFFER_SIZE        (128U >> 1U)

#define FLEXCOM4_USART_WRITE_BUFFER_SIZE            128U
#define FLEXCOM4_USART_9BIT_WRITE_BUFFER_SIZE       (128U >> 1U)

static volatile uint8_t FLEXCOM4_USART_ReadBuffer[FLEXCOM4_USART_READ_BUFFER_SIZE];
static volatile uint8_t FLEXCOM4_USART_WriteBuffer[FLEXCOM4_USART_WRITE_BUFFER_SIZE];

/* LIN related Interrupts */
#define FLEXCOM4_LIN_LINTC_INT_DISABLE()      FLEXCOM4_REGS->FLEX_US_IDR = FLEX_US_IDR_LIN_LINTC_Msk
#define FLEXCOM4_LIN_LINTC_INT_ENABLE()       FLEXCOM4_REGS->FLEX_US_IER = FLEX_US_IER_LIN_LINTC_Msk

#define FLEXCOM4_LIN_LINID_INT_DISABLE()      FLEXCOM4_REGS->FLEX_US_IDR = FLEX_US_IDR_LIN_LINID_Msk
#define FLEXCOM4_LIN_LINID_INT_ENABLE()       FLEXCOM4_REGS->FLEX_US_IER = FLEX_US_IER_LIN_LINID_Msk

#define FLEXCOM4_LIN_LINBK_INT_DISABLE()      FLEXCOM4_REGS->FLEX_US_IDR = FLEX_US_IDR_LIN_LINBK_Msk
#define FLEXCOM4_LIN_LINBK_INT_ENABLE()       FLEXCOM4_REGS->FLEX_US_IER = FLEX_US_IER_LIN_LINBK_Msk

static volatile FLEXCOM_LIN_CALLBACK_OBJECT flexcom4LinObj;

// *****************************************************************************
// *****************************************************************************
// Section: FLEXCOM4 USART Ring Buffer Implementation
// *****************************************************************************
// *****************************************************************************

static volatile FLEXCOM_USART_RING_BUFFER_OBJECT flexcom4UsartObj;

void FLEXCOM4_USART_Initialize( void )
{
    /* Set FLEXCOM USART operating mode */
    FLEXCOM4_REGS->FLEX_MR = FLEX_MR_OPMODE_USART;

    /* Reset FLEXCOM4 USART */
    FLEXCOM4_REGS->FLEX_US_CR = (FLEX_US_CR_RSTRX_Msk | FLEX_US_CR_RSTTX_Msk | FLEX_US_CR_RSTSTA_Msk);

    /* Enable FLEXCOM4 USART */
    FLEXCOM4_REGS->FLEX_US_CR = (FLEX_US_CR_TXEN_Msk | FLEX_US_CR_RXEN_Msk);

    /* Configure FLEXCOM4 USART LIN_SLAVE mode */
    FLEXCOM4_REGS->FLEX_US_MR = (FLEX_US_MR_USCLKS_MCK | FLEX_US_MR_CHRL_8_BIT | FLEX_US_MR_PAR_NO | FLEX_US_MR_NBSTOP_1_BIT | FLEX_US_MR_OVER(0));

    /* Write FLEX_US_MR.USART_MODE to select the LIN mode and the client node configuration. */
    /* LIN client node (USART_MODE = 0xB) */
    FLEXCOM4_REGS->FLEX_US_MR |= FLEX_US_MR_USART_MODE_LIN_SLAVE;

    /* CHKTYP: Checksum Type: 0 LIN 2.0 enhanced checksum
     * DLM: Data Length Mode: The response data length is defined by the DLC field of this register.
     * DLC: Data Length Control: Defines the response data length if DLM = 0, in that case the response data length is equal to DLC+1 bytes.
     * WKUPTYP: Wake-up Signal Type: Setting the LINWKUP bit in the control register sends a LIN 2.0 wake-up signal. */
    FLEXCOM4_REGS->FLEX_US_LINMR = FLEX_US_LINMR_CHKTYP(0x0U) | FLEX_US_LINMR_DLM(0x0U) | FLEX_US_LINMR_DLC(8U) | FLEX_US_LINMR_WKUPTYP(0x0U);

    /* Write FLEX_US_BRGR.CD and FLEX_US_BRGR.FP to configure the baud rate. */
    FLEXCOM4_REGS->FLEX_US_BRGR = FLEX_US_BRGR_CD(259) | FLEX_US_BRGR_FP(3);

    flexcom4UsartObj.rdCallback = NULL;
    flexcom4UsartObj.rdInIndex = 0;
    flexcom4UsartObj.rdOutIndex = 0;
    flexcom4UsartObj.isRdNotificationEnabled = false;
    flexcom4UsartObj.isRdNotifyPersistently = false;
    flexcom4UsartObj.rdThreshold = 0;
    flexcom4UsartObj.rdBufferSize = FLEXCOM4_USART_READ_BUFFER_SIZE;
    flexcom4UsartObj.wrCallback = NULL;
    flexcom4UsartObj.wrInIndex = 0;
    flexcom4UsartObj.wrOutIndex = 0;
    flexcom4UsartObj.isWrNotificationEnabled = false;
    flexcom4UsartObj.isWrNotifyPersistently = false;
    flexcom4UsartObj.wrThreshold = 0;
    flexcom4UsartObj.wrBufferSize = FLEXCOM4_USART_WRITE_BUFFER_SIZE;
    flexcom4UsartObj.errorStatus = FLEXCOM_USART_ERROR_NONE;

    if ((FLEXCOM4_REGS->FLEX_US_MR & FLEX_US_MR_MODE9_Msk) != 0U)
    {
        flexcom4UsartObj.rdBufferSize = FLEXCOM4_USART_9BIT_READ_BUFFER_SIZE;
        flexcom4UsartObj.wrBufferSize = FLEXCOM4_USART_9BIT_WRITE_BUFFER_SIZE;
    }
    else
    {
        flexcom4UsartObj.rdBufferSize = FLEXCOM4_USART_READ_BUFFER_SIZE;
        flexcom4UsartObj.wrBufferSize = FLEXCOM4_USART_WRITE_BUFFER_SIZE;
    }

    /* Enable Read, Overrun, Parity and Framing error interrupts */
    FLEXCOM4_USART_RX_INT_ENABLE();

    /* Enable LIN ID interrupt */
    FLEXCOM4_LIN_LINID_INT_ENABLE();

    /* Enable LIN Transfer Complete interrupt */
    FLEXCOM4_LIN_LINTC_INT_ENABLE();

    /* Enable LIN Break interrupt */
    FLEXCOM4_LIN_LINBK_INT_ENABLE();
}

static void FLEXCOM4_USART_ErrorClear( void )
{
    /* Clear the error flags */
    FLEXCOM4_REGS->FLEX_US_CR = FLEX_US_CR_RSTSTA_Msk;

    /* Flush existing error bytes from the RX FIFO */
    while((FLEXCOM4_REGS->FLEX_US_CSR & FLEX_US_CSR_RXRDY_Msk) != 0U)
    {
        if ((FLEXCOM4_REGS->FLEX_US_MR & FLEX_US_MR_MODE9_Msk) != 0U)
        {
            (void)(FLEXCOM_USART_RHR_9BIT_REG);
        }
        else
        {
            (void)(FLEXCOM_USART_RHR_8BIT_REG);
        }
    }
}

FLEXCOM_USART_ERROR FLEXCOM4_USART_ErrorGet( void )
{
    FLEXCOM_USART_ERROR errors = flexcom4UsartObj.errorStatus;

    flexcom4UsartObj.errorStatus = FLEXCOM_USART_ERROR_NONE;

    /* All errors are cleared, but send the previous error state */
    return errors;
}

static void FLEXCOM4_USART_BaudCalculate(uint32_t srcClkFreq, uint32_t reqBaud, uint8_t overSamp, uint32_t* cd, uint32_t* fp, uint32_t* baudError)
{
    uint32_t actualBaud = 0;

    *cd = srcClkFreq / (reqBaud * 8U * (2U - (uint32_t)overSamp));

    if (*cd > 0U)
    {
        *fp = ((srcClkFreq / (reqBaud * (2U - (uint32_t)overSamp))) - ((*cd) * 8U));
        actualBaud = (srcClkFreq / (((*cd) * 8U) + (*fp))) / (2U - overSamp);
        *baudError = ((100U * actualBaud)/reqBaud) - 100U;
    }
}

bool FLEXCOM4_USART_SerialSetup( FLEXCOM_USART_SERIAL_SETUP *setup, uint32_t srcClkFreq )
{
    uint32_t baud = 0;
    uint32_t overSampVal = 0;
    uint32_t usartMode;
    uint32_t cd0, fp0, cd1, fp1, baudError0, baudError1;
    bool status = false;

    cd0 = fp0 = cd1 = fp1 = baudError0 = baudError1 = 0U;

    if (setup != NULL)
    {
        baud = setup->baudRate;

        if(srcClkFreq == 0U)
        {
            srcClkFreq = FLEXCOM4_USART_FrequencyGet();
        }

        /* Calculate baud register values for 8x/16x oversampling values */

        FLEXCOM4_USART_BaudCalculate(srcClkFreq, baud, 0, &cd0, &fp0, &baudError0);
        FLEXCOM4_USART_BaudCalculate(srcClkFreq, baud, 1, &cd1, &fp1, &baudError1);

        if ( (!(cd0 > 0U && cd0 <= 65535U)) && (!(cd1 > 0U && cd1 <= 65535U)) )
        {
            /* Requested baud cannot be generated with current clock settings */
            return status;
        }

        if ( ((cd0 > 0U) && (cd0 <= 65535U)) && ((cd1 > 0U) && (cd1 <= 65535U)) )
        {
            /* Requested baud can be generated with both 8x and 16x oversampling. Select the one with less % error. */
            if (baudError1 < baudError0)
            {
                cd0 = cd1;
                fp0 = fp1;
                overSampVal = (1UL << FLEX_US_MR_OVER_Pos) & FLEX_US_MR_OVER_Msk;
            }
        }
        else
        {
            /* Requested baud can be generated with either with 8x oversampling or with 16x oversampling. Select valid one. */
            if ((cd1 > 0U) && (cd1 <= 65535U))
            {
                cd0 = cd1;
                fp0 = fp1;
                overSampVal = (1UL << FLEX_US_MR_OVER_Pos) & FLEX_US_MR_OVER_Msk;
            }
        }

        /* Configure FLEXCOM4 USART mode */
        usartMode = FLEXCOM4_REGS->FLEX_US_MR;
        usartMode &= ~(FLEX_US_MR_CHRL_Msk | FLEX_US_MR_MODE9_Msk | FLEX_US_MR_PAR_Msk | FLEX_US_MR_NBSTOP_Msk | FLEX_US_MR_OVER_Msk);
        FLEXCOM4_REGS->FLEX_US_MR = usartMode | ((uint32_t)setup->dataWidth | (uint32_t)setup->parity | (uint32_t)setup->stopBits | overSampVal);

        /* Configure FLEXCOM4 USART Baud Rate */
        FLEXCOM4_REGS->FLEX_US_BRGR = FLEX_US_BRGR_CD(cd0) | FLEX_US_BRGR_FP(fp0);

        if ((FLEXCOM4_REGS->FLEX_US_MR & FLEX_US_MR_MODE9_Msk) != 0U)
        {
            flexcom4UsartObj.rdBufferSize = FLEXCOM4_USART_9BIT_READ_BUFFER_SIZE;
            flexcom4UsartObj.wrBufferSize = FLEXCOM4_USART_9BIT_WRITE_BUFFER_SIZE;
        }
        else
        {
            flexcom4UsartObj.rdBufferSize = FLEXCOM4_USART_READ_BUFFER_SIZE;
            flexcom4UsartObj.wrBufferSize = FLEXCOM4_USART_WRITE_BUFFER_SIZE;
        }

        status = true;
    }

    return status;
}

/* This routine is only called from ISR. Hence do not disable/enable USART interrupts. */
static bool FLEXCOM4_USART_TxPullByte(void* pWrData)
{
    bool isSuccess = false;
    uint32_t wrOutIndex = flexcom4UsartObj.wrOutIndex;
    uint32_t wrInIndex = flexcom4UsartObj.wrInIndex;
    uint8_t* pWrByte = (uint8_t*)pWrData;
    if (wrOutIndex != wrInIndex)
    {
        if ((FLEXCOM4_REGS->FLEX_US_MR & FLEX_US_MR_MODE9_Msk) != 0U)
        {
            uint32_t wrOutIdx = wrOutIndex << 1U;
            pWrByte[0] = FLEXCOM4_USART_WriteBuffer[wrOutIdx];
            pWrByte[1] = FLEXCOM4_USART_WriteBuffer[wrOutIdx + 1U];
        }
        else
        {
            *pWrByte = FLEXCOM4_USART_WriteBuffer[wrOutIndex];
        }
        wrOutIndex++;

        if (wrOutIndex >= flexcom4UsartObj.wrBufferSize)
        {
            wrOutIndex = 0U;
        }

        flexcom4UsartObj.wrOutIndex = wrOutIndex;
        isSuccess = true;
    }
    return isSuccess;
}

static inline bool FLEXCOM4_USART_TxPushByte(uint16_t wrByte)
{
    uint32_t tempInIndex;
    uint32_t wrOutIndex;
    uint32_t wrInIndex;
    bool isSuccess = false;

    /* Take a snapshot of indices to avoid creation of critical section */
    wrOutIndex = flexcom4UsartObj.wrOutIndex;
    wrInIndex = flexcom4UsartObj.wrInIndex;

    tempInIndex = wrInIndex + 1U;
    if (tempInIndex >= flexcom4UsartObj.wrBufferSize)
    {
        tempInIndex = 0U;
    }
    if (tempInIndex != wrOutIndex)
    {
        if ((FLEXCOM4_REGS->FLEX_US_MR & FLEX_US_MR_MODE9_Msk) != 0U)
        {
            uint32_t wrInIdx = wrInIndex << 1U;
            FLEXCOM4_USART_WriteBuffer[wrInIdx] = (uint8_t)wrByte;
            FLEXCOM4_USART_WriteBuffer[wrInIdx + 1U] = (uint8_t)(wrByte >> 8U);
        }
        else
        {
            FLEXCOM4_USART_WriteBuffer[wrInIndex] = (uint8_t)wrByte;
        }

        flexcom4UsartObj.wrInIndex = tempInIndex;
        isSuccess = true;
    }
    else
    {
        /* Queue is full. Report Error. */
    }
    return isSuccess;
}

/* This routine is only called from ISR. Hence do not disable/enable USART interrupts. */
static void FLEXCOM4_USART_SendWriteNotification(void)
{
    uint32_t nFreeWrBufferCount;

    if (flexcom4UsartObj.isWrNotificationEnabled == true)
    {
        nFreeWrBufferCount = FLEXCOM4_USART_WriteFreeBufferCountGet();

        if(flexcom4UsartObj.wrCallback != NULL)
        {
            uintptr_t wrContext = flexcom4UsartObj.wrContext;

            if (flexcom4UsartObj.isWrNotifyPersistently == true)
            {
                if (nFreeWrBufferCount >= flexcom4UsartObj.wrThreshold)
                {
                    flexcom4UsartObj.wrCallback(FLEXCOM_USART_EVENT_WRITE_THRESHOLD_REACHED, wrContext);
                }
            }
            else
            {
                if (nFreeWrBufferCount == flexcom4UsartObj.wrThreshold)
                {
                    flexcom4UsartObj.wrCallback(FLEXCOM_USART_EVENT_WRITE_THRESHOLD_REACHED, wrContext);
                }
            }
        }
    }
}

static size_t FLEXCOM4_USART_WritePendingBytesGet(void)
{
    size_t nPendingTxBytes;

    /* Take a snapshot of indices to avoid creation of critical section */

    uint32_t wrOutIndex = flexcom4UsartObj.wrOutIndex;
    uint32_t wrInIndex = flexcom4UsartObj.wrInIndex;

    if ( wrInIndex >=  wrOutIndex)
    {
        nPendingTxBytes =  wrInIndex -  wrOutIndex;
    }
    else
    {
        nPendingTxBytes =  (flexcom4UsartObj.wrBufferSize -  wrOutIndex) + wrInIndex;
    }

    return nPendingTxBytes;
}

size_t FLEXCOM4_USART_WriteCountGet(void)
{
    size_t nPendingTxBytes;

    nPendingTxBytes = FLEXCOM4_USART_WritePendingBytesGet();

    return nPendingTxBytes;
}

size_t FLEXCOM4_USART_Write(uint8_t* pWrBuffer, const size_t size )
{
    size_t nBytesWritten  = 0;

    while (nBytesWritten < size)
    {
        if ((FLEXCOM4_REGS->FLEX_US_MR & FLEX_US_MR_MODE9_Msk) != 0U)
        {
            uint16_t halfWordData = (uint16_t)(pWrBuffer[(2U * nBytesWritten) + 1U]);
            halfWordData <<= 8U;
            halfWordData |= (uint16_t)pWrBuffer[2U * nBytesWritten];
            if (FLEXCOM4_USART_TxPushByte(halfWordData) == true)
            {
                nBytesWritten++;
            }
            else
            {
                /* Queue is full, exit the loop */
                break;
            }
        }
        else
        {
            if (FLEXCOM4_USART_TxPushByte(pWrBuffer[nBytesWritten]) == true)
            {
                nBytesWritten++;
            }
            else
            {
                /* Queue is full, exit the loop */
                break;
            }
        }
    }

    /* Check if any data is pending for transmission */
    if (FLEXCOM4_USART_WritePendingBytesGet() > 0U)
    {
        /* Enable TX interrupt as data is pending for transmission */
        FLEXCOM4_USART_TX_INT_ENABLE();
    }

    return nBytesWritten;
}

size_t FLEXCOM4_USART_WriteFreeBufferCountGet(void)
{
    return (flexcom4UsartObj.wrBufferSize - 1U) - FLEXCOM4_USART_WriteCountGet();
}

size_t FLEXCOM4_USART_WriteBufferSizeGet(void)
{
    return (flexcom4UsartObj.wrBufferSize - 1U);
}

bool FLEXCOM4_USART_TransmitComplete(void)
{
    bool status = false;

    if ((FLEXCOM4_REGS->FLEX_US_CSR & FLEX_US_CSR_TXEMPTY_Msk) != 0U)
    {
        status = true;
    }

    return status;
}

bool FLEXCOM4_USART_WriteNotificationEnable(bool isEnabled, bool isPersistent)
{
    bool previousStatus = flexcom4UsartObj.isWrNotificationEnabled;

    flexcom4UsartObj.isWrNotificationEnabled = isEnabled;

    flexcom4UsartObj.isWrNotifyPersistently = isPersistent;

    return previousStatus;
}

void FLEXCOM4_USART_WriteThresholdSet(uint32_t nBytesThreshold)
{
    if (nBytesThreshold > 0U)
    {
        flexcom4UsartObj.wrThreshold = nBytesThreshold;
    }
}

void FLEXCOM4_USART_WriteCallbackRegister( FLEXCOM_USART_RING_BUFFER_CALLBACK callback, uintptr_t context)
{
    flexcom4UsartObj.wrCallback = callback;

    flexcom4UsartObj.wrContext = context;
}

/* This routine is only called from ISR. Hence do not disable/enable USART interrupts. */
static inline bool FLEXCOM4_USART_RxPushByte(uint16_t rdByte)
{
    uint32_t tempInIndex;
    bool isSuccess = false;

    tempInIndex = flexcom4UsartObj.rdInIndex + 1U;

    if (tempInIndex >= flexcom4UsartObj.rdBufferSize)
    {
        tempInIndex = 0U;
    }

    if (tempInIndex == flexcom4UsartObj.rdOutIndex)
    {
        /* Queue is full - Report it to the application. Application gets a chance to free up space by reading data out from the RX ring buffer */
        if(flexcom4UsartObj.rdCallback != NULL)
        {
            uintptr_t rdContext = flexcom4UsartObj.rdContext;

            flexcom4UsartObj.rdCallback(FLEXCOM_USART_EVENT_READ_BUFFER_FULL, rdContext);

            /* Read the indices again in case application has freed up space in RX ring buffer */
            tempInIndex = flexcom4UsartObj.rdInIndex + 1U;

            if (tempInIndex >= flexcom4UsartObj.rdBufferSize)
            {
                tempInIndex = 0U;
            }
        }
    }

    /* Attempt to push the data into the ring buffer */
    if (tempInIndex != flexcom4UsartObj.rdOutIndex)
    {
        uint32_t rdInIdx;

        if ((FLEXCOM4_REGS->FLEX_US_MR & FLEX_US_MR_MODE9_Msk) != 0U)
        {
            rdInIdx = flexcom4UsartObj.rdInIndex << 1U;
            FLEXCOM4_USART_ReadBuffer[rdInIdx] = (uint8_t)rdByte;
            FLEXCOM4_USART_ReadBuffer[rdInIdx + 1U] = (uint8_t)(rdByte >> 8U);
        }
        else
        {
            rdInIdx = flexcom4UsartObj.rdInIndex;
            FLEXCOM4_USART_ReadBuffer[rdInIdx] = (uint8_t)rdByte;
        }

        flexcom4UsartObj.rdInIndex = tempInIndex;
        isSuccess = true;
    }
    else
    {
        /* Queue is full. Data will be lost. */
    }

    return isSuccess;
}

/* This routine is only called from ISR. Hence do not disable/enable USART interrupts. */
static void FLEXCOM4_USART_SendReadNotification(void)
{
    uint32_t nUnreadBytesAvailable;

    if (flexcom4UsartObj.isRdNotificationEnabled == true)
    {
        nUnreadBytesAvailable = FLEXCOM4_USART_ReadCountGet();

        if(flexcom4UsartObj.rdCallback != NULL)
        {
            uintptr_t rdContext = flexcom4UsartObj.rdContext;

            if (flexcom4UsartObj.isRdNotifyPersistently == true)
            {
                if (nUnreadBytesAvailable >= flexcom4UsartObj.rdThreshold)
                {
                    flexcom4UsartObj.rdCallback(FLEXCOM_USART_EVENT_READ_THRESHOLD_REACHED, rdContext);
                }
            }
            else
            {
                if (nUnreadBytesAvailable == flexcom4UsartObj.rdThreshold)
                {
                    flexcom4UsartObj.rdCallback(FLEXCOM_USART_EVENT_READ_THRESHOLD_REACHED, rdContext);
                }
            }
        }
    }
}

size_t FLEXCOM4_USART_Read(uint8_t* pRdBuffer, const size_t size)
{
    size_t nBytesRead = 0;
    uint32_t rdOutIndex = 0;
    uint32_t rdInIndex = 0;


    /* Take a snapshot of indices to avoid creation of critical section */
    rdOutIndex = flexcom4UsartObj.rdOutIndex;
    rdInIndex = flexcom4UsartObj.rdInIndex;

    while (nBytesRead < size)
    {
        if (rdOutIndex != rdInIndex)
        {
            if ((FLEXCOM4_REGS->FLEX_US_MR & FLEX_US_MR_MODE9_Msk) != 0U)
            {
                uint32_t rdOutIdx = rdOutIndex << 1U;
                uint32_t nBytesReadIdx = nBytesRead << 1U;
                pRdBuffer[nBytesReadIdx] = FLEXCOM4_USART_ReadBuffer[rdOutIdx];
                pRdBuffer[nBytesReadIdx + 1U] = FLEXCOM4_USART_ReadBuffer[rdOutIdx + 1U];
                nBytesRead++;
                rdOutIndex++;
            }
            else
            {
                pRdBuffer[nBytesRead++] = FLEXCOM4_USART_ReadBuffer[rdOutIndex];
                rdOutIndex++;
            }

            if (rdOutIndex >= flexcom4UsartObj.rdBufferSize)
            {
                rdOutIndex = 0U;
            }
        }
        else
        {
            /* No more data available in the RX buffer */
            break;
        }
    }

    flexcom4UsartObj.rdOutIndex = rdOutIndex;

    return nBytesRead;
}

size_t FLEXCOM4_USART_ReadCountGet(void)
{
    size_t nUnreadBytesAvailable;
    uint32_t rdOutIndex;
    uint32_t rdInIndex;


    /* Take a snapshot of indices to avoid creation of critical section */
    rdOutIndex = flexcom4UsartObj.rdOutIndex;
    rdInIndex = flexcom4UsartObj.rdInIndex;

    if ( rdInIndex >=  rdOutIndex)
    {
        nUnreadBytesAvailable =  rdInIndex -  rdOutIndex;
    }
    else
    {
        nUnreadBytesAvailable =  (flexcom4UsartObj.rdBufferSize -  rdOutIndex) + rdInIndex;
    }

    return nUnreadBytesAvailable;
}

size_t FLEXCOM4_USART_ReadFreeBufferCountGet(void)
{
    return (flexcom4UsartObj.rdBufferSize - 1U) - FLEXCOM4_USART_ReadCountGet();
}

size_t FLEXCOM4_USART_ReadBufferSizeGet(void)
{
    return (flexcom4UsartObj.rdBufferSize - 1U);
}


bool FLEXCOM4_USART_ReadNotificationEnable(bool isEnabled, bool isPersistent)
{
    bool previousStatus = flexcom4UsartObj.isRdNotificationEnabled;

    flexcom4UsartObj.isRdNotificationEnabled = isEnabled;

    flexcom4UsartObj.isRdNotifyPersistently = isPersistent;


    return previousStatus;
}

void FLEXCOM4_USART_ReadThresholdSet(uint32_t nBytesThreshold)
{
    if (nBytesThreshold > 0U)
    {
        flexcom4UsartObj.rdThreshold = nBytesThreshold;

    }
}

void FLEXCOM4_USART_ReadCallbackRegister( FLEXCOM_USART_RING_BUFFER_CALLBACK callback, uintptr_t context)
{
    flexcom4UsartObj.rdCallback = callback;

    flexcom4UsartObj.rdContext = context;
}

static void __attribute__((used)) FLEXCOM4_USART_ISR_RX_Handler( void )
{
    uint16_t rdData = 0;

    /* Keep reading until there is a character availabe in the RX FIFO */
    while(FLEX_US_CSR_RXRDY_Msk == (FLEXCOM4_REGS->FLEX_US_CSR & FLEX_US_CSR_RXRDY_Msk))
    {
        if ((FLEXCOM4_REGS->FLEX_US_MR & FLEX_US_MR_MODE9_Msk) != 0U)
        {
            rdData = FLEXCOM_USART_RHR_9BIT_REG & (uint16_t)FLEX_US_RHR_RXCHR_Msk;
        }
        else
        {
            rdData = (uint8_t)FLEXCOM_USART_RHR_8BIT_REG;
        }

        if (FLEXCOM4_USART_RxPushByte( rdData ) == true)
        {
            FLEXCOM4_USART_SendReadNotification();
        }
        else
        {
            /* UART RX buffer is full */
        }
    }


}

static void __attribute__((used)) FLEXCOM4_USART_ISR_TX_Handler( void )
{
    uint16_t wrByte;

    /* Keep writing to the TX FIFO as long as there is space */
    while(FLEX_US_CSR_TXRDY_Msk == (FLEXCOM4_REGS->FLEX_US_CSR & FLEX_US_CSR_TXRDY_Msk))
    {
        if (FLEXCOM4_USART_TxPullByte(&wrByte) == true)
        {
            if ((FLEXCOM4_REGS->FLEX_US_MR & FLEX_US_MR_MODE9_Msk) != 0U)
            {
                FLEXCOM_USART_THR_9BIT_REG = wrByte & (uint16_t)FLEX_US_THR_TXCHR_Msk;
            }
            else
            {
                FLEXCOM_USART_THR_8BIT_REG = (uint8_t)wrByte;
            }

            /* Send notification */
            FLEXCOM4_USART_SendWriteNotification();
        }
        else
        {
            /* Nothing to transmit. Disable the data register empty interrupt. */
            FLEXCOM4_USART_TX_INT_DISABLE();
            break;
        }
    }

}

void __attribute__((used)) FLEXCOM4_InterruptHandler( void )
{
    /* Channel status */
    uint32_t channelStatus = FLEXCOM4_REGS->FLEX_US_CSR;

    /* Error status */
    uint32_t errorStatus = (channelStatus & (FLEX_US_CSR_OVRE_Msk | FLEX_US_CSR_LIN_FRAME_Msk | FLEX_US_CSR_LIN_PARE_Msk | FLEX_US_CSR_LIN_TIMEOUT_Msk
                                           | FLEX_US_CSR_LIN_LINBE_Msk | FLEX_US_CSR_LIN_LINISFE_Msk | FLEX_US_CSR_LIN_LINIPE_Msk
                                           | FLEX_US_CSR_LIN_LINCE_Msk | FLEX_US_CSR_LIN_LINSNRE_Msk | FLEX_US_CSR_LIN_LINSTE_Msk | FLEX_US_CSR_LIN_LINHTE_Msk));


    if(errorStatus != 0U)
    {
        /* Save the error so that it can be reported when application calls the FLEXCOM4_USART_ErrorGet() API */
        flexcom4UsartObj.errorStatus = (FLEXCOM_USART_ERROR)errorStatus;

        /* Clear the error flags and flush out the error bytes */
        FLEXCOM4_USART_ErrorClear();

        /* USART errors are normally associated with the receiver, hence calling receiver context */
        if( flexcom4UsartObj.rdCallback != NULL )
        {
            uintptr_t rdContext = flexcom4UsartObj.rdContext;

            flexcom4UsartObj.rdCallback(FLEXCOM_USART_EVENT_READ_ERROR, rdContext);
        }
    }

    /* Receiver status. RX interrupt is never disabled. */
    if ((channelStatus & FLEX_US_CSR_RXRDY_Msk) != 0U)
    {
        FLEXCOM4_USART_ISR_RX_Handler();
    }

    /* Transmitter status */
    if( ((channelStatus & FLEX_US_CSR_TXRDY_Msk) != 0U) && ((FLEXCOM4_REGS->FLEX_US_IMR & FLEX_US_IMR_TXRDY_Msk) != 0U) )
    {
        FLEXCOM4_USART_ISR_TX_Handler();
    }

    /* LIN Break Detected */
    if((FLEXCOM4_REGS->FLEX_US_CSR & FLEX_US_CSR_LIN_LINBK_Msk) != 0U)
    {
        FLEXCOM4_REGS->FLEX_US_CR = FLEX_US_CR_RSTSTA_Msk;

        if( flexcom4LinObj.breakCallback != NULL)
        {
            uintptr_t breakContext = flexcom4LinObj.breakContext;
            flexcom4LinObj.breakCallback(breakContext);
        }
    }

    /* LIN ID Receive */
    if((FLEXCOM4_REGS->FLEX_US_CSR & FLEX_US_CSR_LIN_LINID_Msk) != 0U)
    {
        /* Clear the status register */
        FLEXCOM4_REGS->FLEX_US_CR = FLEX_US_CR_RSTSTA_Msk;

        /* Check if the ID callback function is set */
        if( flexcom4LinObj.idCallback != NULL)
        {
            /* Retrieve the context for the ID callback */
            uintptr_t idContext = flexcom4LinObj.idContext;
            /* Check if the ID callback function is set */
            flexcom4LinObj.idCallback(idContext);
        }
    }

    /* LIN Transfer Complete */
    if((FLEXCOM4_REGS->FLEX_US_CSR & FLEX_US_CSR_LIN_LINTC_Msk) != 0U)
    {
        /* Clear the status register */
        FLEXCOM4_REGS->FLEX_US_CR = FLEX_US_CR_RSTSTA_Msk;

        /* Check if the transfer callback function is set */
        if( flexcom4LinObj.tranferCallback != NULL)
        {
            /* Retrieve the context for the transfer callback */
            uintptr_t transferContext = flexcom4LinObj.tranferContext;
            /* Call the transfer callback function with the context */
            flexcom4LinObj.tranferCallback(transferContext);
        }
    }
}

/* Sets the LIN (Local Interconnect Network) node action for FLEXCOM4. */
void FLEXCOM4_LIN_NodeActionSet( FLEXCOM_LIN_NACT action )
{
    /* Clear the current node action bits in the LIN Mode Register (LINMR). */
    FLEXCOM4_REGS->FLEX_US_LINMR &= ~(FLEX_US_LINMR_NACT_Msk);
    /* Set the new node action by applying the provided action value. */
    FLEXCOM4_REGS->FLEX_US_LINMR |= FLEX_US_LINMR_NACT(action);
}

/* Writes an identifier to the LIN Identifier Register (LINIR) of FLEXCOM4. */
bool FLEXCOM4_LIN_IdentifierWrite( uint8_t id)
{
    bool status = false;

    /* Reset the status bits in the Control Register (CR). */
    FLEXCOM4_REGS->FLEX_US_CR = FLEX_US_CR_RSTSTA_Msk;

    /* Check if the Transmit Ready (TXRDY) flag is set in the Channel Status Register (CSR). */
    if((FLEXCOM4_REGS->FLEX_US_CSR & FLEX_US_CSR_TXRDY_Msk) != 0U)
    {
        /* If TXRDY is set, write the identifier to the LIN Identifier Register (LINIR). */
        FLEXCOM4_REGS->FLEX_US_LINIR = id;
        /* Set the status to true indicating the identifier was successfully written. */
        status = true;
    }

    return status;
}

/* Reads and returns the LIN identifier from the FLEXCOM4 register. */
uint8_t FLEXCOM4_LIN_IdentifierRead(void)
{
    return (uint8_t)(FLEXCOM4_REGS->FLEX_US_LINIR);
}

/* Enables or disables the parity check for LIN communication. */
void FLEXCOM4_LIN_ParityEnable(bool parityEnable)
{
    if(parityEnable == true)
    {
        /* Clear the PARDIS bit to enable parity. */
        FLEXCOM4_REGS->FLEX_US_LINMR &= ~FLEX_US_LINMR_PARDIS_Msk;
    }
    else
    {
        /* Set the PARDIS bit to disable parity. */
        FLEXCOM4_REGS->FLEX_US_LINMR |= FLEX_US_LINMR_PARDIS_Msk;
    }
}

/* Enables or disables the checksum for LIN communication. */
void FLEXCOM4_LIN_ChecksumEnable(bool checksumEnable)
{
    if(checksumEnable == true)
    {
        /* Clear the CHKDIS bit to enable checksum. */
        FLEXCOM4_REGS->FLEX_US_LINMR &= ~FLEX_US_LINMR_CHKDIS_Msk;
    }
    else
    {
        /* Set the CHKDIS bit to disable checksum. */
        FLEXCOM4_REGS->FLEX_US_LINMR |= FLEX_US_LINMR_CHKDIS_Msk;
    }
}

/* Sets the type of checksum to be used in LIN communication. */
void FLEXCOM4_LIN_ChecksumTypeSet(FLEXCOM_LIN_CHECKSUM_TYPE checksumType)
{
    /* Clear Existing Checksum Type */
    FLEXCOM4_REGS->FLEX_US_LINMR &= ~FLEX_US_LINMR_CHKTYP_Msk;
    /* Set New Checksum Type */
    FLEXCOM4_REGS->FLEX_US_LINMR |= (uint32_t)checksumType;
}

/* Sets the data length mode for LIN communication. */
void FLEXCOM4_LIN_DataLenModeSet(FLEXCOM_LIN_DATA_LEN dataLenMode)
{
    FLEXCOM4_REGS->FLEX_US_LINMR &= ~FLEX_US_LINMR_DLM_Msk;
    FLEXCOM4_REGS->FLEX_US_LINMR |= (uint32_t)dataLenMode;
}

/* Sets the response data length for LIN communication. */
void FLEXCOM4_LIN_ResponseDataLenSet(uint8_t len)
{
    FLEXCOM4_REGS->FLEX_US_LINMR &= ~FLEX_US_LINMR_DLC_Msk;
    FLEXCOM4_REGS->FLEX_US_LINMR |= FLEX_US_LINMR_DLC((uint32_t)len-1U);
}

/* Checks if the LIN transfer is complete and returns the status. */
/* If a LIN transfer has completed, it returns 1; otherwise, it returns 0. */
uint8_t FLEXCOM4_LIN_TransferComplete(void)
{
    /* Check if the LIN transfer complete status bit is set in the status register */
    return (uint8_t)((FLEXCOM4_REGS->FLEX_US_CSR & FLEX_US_CSR_LIN_LINTC_Msk) > 0U);
}

/* Registers a callback for LIN identifier events. */
void FLEXCOM4_LINIdCallbackRegister(FLEXCOM_LIN_CALLBACK callback, uintptr_t context)
{
    flexcom4LinObj.idCallback = callback;
    flexcom4LinObj.idContext = context;
}

/* Registers a callback for LIN transfer complete events. */
void FLEXCOM4_LINTcCallbackRegister(FLEXCOM_LIN_CALLBACK callback, uintptr_t context)
{
    flexcom4LinObj.tranferCallback = callback;
    flexcom4LinObj.tranferContext = context;
}

/* Registers a callback for LIN break events. */
void FLEXCOM4_LINBreakCallbackRegister(FLEXCOM_LIN_CALLBACK callback, uintptr_t context)
{
    flexcom4LinObj.breakCallback = callback;
    flexcom4LinObj.breakContext = context;
}
