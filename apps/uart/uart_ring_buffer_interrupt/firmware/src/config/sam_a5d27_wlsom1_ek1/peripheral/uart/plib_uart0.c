/*******************************************************************************
  UART0 PLIB

  Company:
    Microchip Technology Inc.

  File Name:
    plib_uart0.c

  Summary:
    UART0 PLIB Implementation File

  Description:
    None

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

#include "device.h"
#include "plib_uart0.h"
#include "interrupts.h"

// *****************************************************************************
// *****************************************************************************
// Section: UART0 Implementation
// *****************************************************************************
// *****************************************************************************

UART_RING_BUFFER_OBJECT uart0Obj;

#define UART0_READ_BUFFER_SIZE      20
/* Disable Read, Overrun, Parity and Framing error interrupts */
#define UART0_RX_INT_DISABLE()      UART0_REGS->UART_IDR = (UART_IDR_RXRDY_Msk | UART_IDR_FRAME_Msk | UART_IDR_PARE_Msk | UART_IDR_OVRE_Msk);
/* Enable Read, Overrun, Parity and Framing error interrupts */
#define UART0_RX_INT_ENABLE()       UART0_REGS->UART_IER = (UART_IER_RXRDY_Msk | UART_IER_FRAME_Msk | UART_IER_PARE_Msk | UART_IER_OVRE_Msk);

static uint8_t UART0_ReadBuffer[UART0_READ_BUFFER_SIZE];

#define UART0_WRITE_BUFFER_SIZE     128
#define UART0_TX_INT_DISABLE()      UART0_REGS->UART_IDR = UART_IDR_TXEMPTY_Msk;
#define UART0_TX_INT_ENABLE()       UART0_REGS->UART_IER = UART_IER_TXEMPTY_Msk;

static uint8_t UART0_WriteBuffer[UART0_WRITE_BUFFER_SIZE];

void UART0_Initialize( void )
{
    /* Reset UART0 */
    UART0_REGS->UART_CR = (UART_CR_RSTRX_Msk | UART_CR_RSTTX_Msk | UART_CR_RSTSTA_Msk);

    /* Enable UART0 */
    UART0_REGS->UART_CR = (UART_CR_TXEN_Msk | UART_CR_RXEN_Msk);

    /* Configure UART0 mode */
    UART0_REGS->UART_MR = ((UART_MR_BRSRCCK_PERIPH_CLK) | (UART_MR_PAR_NO) | (0 << UART_MR_FILTER_Pos));

    /* Configure UART0 Baud Rate */
    UART0_REGS->UART_BRGR = UART_BRGR_CD(45);

    /* Initialize instance object */
    uart0Obj.rdCallback = NULL;
    uart0Obj.rdInIndex = 0;
	uart0Obj.rdOutIndex = 0;
    uart0Obj.isRdNotificationEnabled = false;
    uart0Obj.isRdNotifyPersistently = false;
    uart0Obj.rdThreshold = 0;
    uart0Obj.wrCallback = NULL;
    uart0Obj.wrInIndex = 0;
	uart0Obj.wrOutIndex = 0;
    uart0Obj.isWrNotificationEnabled = false;
    uart0Obj.isWrNotifyPersistently = false;
    uart0Obj.wrThreshold = 0;

    /* Enable receive interrupt */
    UART0_RX_INT_ENABLE()
}

bool UART0_SerialSetup( UART_SERIAL_SETUP *setup, uint32_t srcClkFreq )
{
    bool status = false;
    uint32_t baud;
    uint32_t brgVal = 0;
    uint32_t uartMode;

    if (setup != NULL)
    {
        baud = setup->baudRate;

        if(srcClkFreq == 0)
        {
            srcClkFreq = UART0_FrequencyGet();
        }

        /* Calculate BRG value */
        brgVal = srcClkFreq / (16 * baud);

        /* If the target baud rate is acheivable using this clock */
        if (brgVal <= 65535)
        {
            /* Configure UART0 mode */
            uartMode = UART0_REGS->UART_MR;
            uartMode &= ~UART_MR_PAR_Msk;
            UART0_REGS->UART_MR = uartMode | setup->parity ;

            /* Configure UART0 Baud Rate */
            UART0_REGS->UART_BRGR = UART_BRGR_CD(brgVal);

            status = true;
        }
    }

    return status;
}

static void UART0_ErrorClear( void )
{
    uint8_t dummyData = 0u;

    UART0_REGS->UART_CR = UART_CR_RSTSTA_Msk;

    /* Flush existing error bytes from the RX FIFO */
    while( UART_SR_RXRDY_Msk == (UART0_REGS->UART_SR & UART_SR_RXRDY_Msk) )
    {
        dummyData = (UART0_REGS->UART_RHR & UART_RHR_RXCHR_Msk);
    }

    /* Ignore the warning */
    (void)dummyData;
}

UART_ERROR UART0_ErrorGet( void )
{
    UART_ERROR errors = UART_ERROR_NONE;
    uint32_t status = UART0_REGS->UART_SR;

    errors = (UART_ERROR)(status & (UART_SR_OVRE_Msk | UART_SR_PARE_Msk | UART_SR_FRAME_Msk));

    if(errors != UART_ERROR_NONE)
    {
        UART0_ErrorClear();
    }

    /* All errors are cleared, but send the previous error state */
    return errors;
}

/* This routine is only called from ISR. Hence do not disable/enable USART interrupts. */
static inline bool UART0_RxPushByte(uint8_t rdByte)
{
    uint32_t tempInIndex;
    bool isSuccess = false;

    tempInIndex = uart0Obj.rdInIndex + 1;

    if (tempInIndex >= UART0_READ_BUFFER_SIZE)
    {
        tempInIndex = 0;
    }

    if (tempInIndex == uart0Obj.rdOutIndex)
    {
        /* Queue is full - Report it to the application. Application gets a chance to free up space by reading data out from the RX ring buffer */
        if(uart0Obj.rdCallback != NULL)
        {
            uart0Obj.rdCallback(UART_EVENT_READ_BUFFER_FULL, uart0Obj.rdContext);
        }

        /* Read the indices again in case application has freed up space in RX ring buffer */
        tempInIndex = uart0Obj.rdInIndex + 1;

        if (tempInIndex >= UART0_READ_BUFFER_SIZE)
        {
            tempInIndex = 0;
        }

    }

    if (tempInIndex != uart0Obj.rdOutIndex)
    {
        UART0_ReadBuffer[uart0Obj.rdInIndex] = rdByte;
        uart0Obj.rdInIndex = tempInIndex;
        isSuccess = true;
    }
    else
    {
        /* Queue is full. Data will be lost. */
    }

    return isSuccess;
}

/* This routine is only called from ISR. Hence do not disable/enable USART interrupts. */
static void UART0_ReadNotificationSend(void)
{
    uint32_t nUnreadBytesAvailable;

    if (uart0Obj.isRdNotificationEnabled == true)
    {
        nUnreadBytesAvailable = UART0_ReadCountGet();

        if(uart0Obj.rdCallback != NULL)
        {
            if (uart0Obj.isRdNotifyPersistently == true)
            {
                if (nUnreadBytesAvailable >= uart0Obj.rdThreshold)
                {
                    uart0Obj.rdCallback(UART_EVENT_READ_THRESHOLD_REACHED, uart0Obj.rdContext);
                }
            }
            else
            {
                if (nUnreadBytesAvailable == uart0Obj.rdThreshold)
                {
                    uart0Obj.rdCallback(UART_EVENT_READ_THRESHOLD_REACHED, uart0Obj.rdContext);
                }
            }
        }
    }
}

size_t UART0_Read(uint8_t* pRdBuffer, const size_t size)
{
    size_t nBytesRead = 0;
	uint32_t rdOutIndex;
	uint32_t rdInIndex;

    while (nBytesRead < size)
    {
        UART0_RX_INT_DISABLE();
		
		rdOutIndex = uart0Obj.rdOutIndex;
		rdInIndex = uart0Obj.rdInIndex;

        if (rdOutIndex != rdInIndex)
        {
            pRdBuffer[nBytesRead++] = UART0_ReadBuffer[uart0Obj.rdOutIndex++];

            if (uart0Obj.rdOutIndex >= UART0_READ_BUFFER_SIZE)
            {
                uart0Obj.rdOutIndex = 0;
            }
            UART0_RX_INT_ENABLE();
        }
        else
        {
            UART0_RX_INT_ENABLE();
            break;
        }
    }

    return nBytesRead;
}

size_t UART0_ReadCountGet(void)
{
    size_t nUnreadBytesAvailable;
	uint32_t rdInIndex;
	uint32_t rdOutIndex;
    
	/* Take  snapshot of indices to avoid creation of critical section */
	rdInIndex = uart0Obj.rdInIndex;
	rdOutIndex = uart0Obj.rdOutIndex;

    if ( rdInIndex >=  rdOutIndex)
    {
        nUnreadBytesAvailable =  rdInIndex - rdOutIndex;
    }
    else
    {
        nUnreadBytesAvailable =  (UART0_READ_BUFFER_SIZE -  rdOutIndex) + rdInIndex;
    }
    
    return nUnreadBytesAvailable;
}

size_t UART0_ReadFreeBufferCountGet(void)
{
    return (UART0_READ_BUFFER_SIZE - 1) - UART0_ReadCountGet();
}

size_t UART0_ReadBufferSizeGet(void)
{
    return (UART0_READ_BUFFER_SIZE - 1);
}

bool UART0_ReadNotificationEnable(bool isEnabled, bool isPersistent)
{
    bool previousStatus = uart0Obj.isRdNotificationEnabled;

    uart0Obj.isRdNotificationEnabled = isEnabled;

    uart0Obj.isRdNotifyPersistently = isPersistent;

    return previousStatus;
}

void UART0_ReadThresholdSet(uint32_t nBytesThreshold)
{
    if (nBytesThreshold > 0)
    {
        uart0Obj.rdThreshold = nBytesThreshold;
    }
}

void UART0_ReadCallbackRegister( UART_RING_BUFFER_CALLBACK callback, uintptr_t context)
{
    uart0Obj.rdCallback = callback;

    uart0Obj.rdContext = context;
}

/* This routine is only called from ISR. Hence do not disable/enable USART interrupts. */
static bool UART0_TxPullByte(uint8_t* pWrByte)
{
    bool isSuccess = false;
	uint32_t wrOutIndex = uart0Obj.wrOutIndex;
	uint32_t wrInIndex = uart0Obj.wrInIndex;

    if (wrOutIndex != wrInIndex)
    {
        *pWrByte = UART0_WriteBuffer[uart0Obj.wrOutIndex++];

        if (uart0Obj.wrOutIndex >= UART0_WRITE_BUFFER_SIZE)
        {
            uart0Obj.wrOutIndex = 0;
        }
        isSuccess = true;
    }

    return isSuccess;
}

static inline bool UART0_TxPushByte(uint8_t wrByte)
{
    uint32_t tempInIndex;
    bool isSuccess = false;

    tempInIndex = uart0Obj.wrInIndex + 1;

    if (tempInIndex >= UART0_WRITE_BUFFER_SIZE)
    {
        tempInIndex = 0;
    }
    if (tempInIndex != uart0Obj.wrOutIndex)
    {
        UART0_WriteBuffer[uart0Obj.wrInIndex] = wrByte;
        uart0Obj.wrInIndex = tempInIndex;
        isSuccess = true;
    }
    else
    {
        /* Queue is full. Report Error. */
    }

    return isSuccess;
}

/* This routine is only called from ISR. Hence do not disable/enable USART interrupts. */
static void UART0_WriteNotificationSend(void)
{
    uint32_t nFreeWrBufferCount;

    if (uart0Obj.isWrNotificationEnabled == true)
    {
        nFreeWrBufferCount = UART0_WriteFreeBufferCountGet();

        if(uart0Obj.wrCallback != NULL)
        {
            if (uart0Obj.isWrNotifyPersistently == true)
            {
                if (nFreeWrBufferCount >= uart0Obj.wrThreshold)
                {
                    uart0Obj.wrCallback(UART_EVENT_WRITE_THRESHOLD_REACHED, uart0Obj.wrContext);
                }
            }
            else
            {
                if (nFreeWrBufferCount == uart0Obj.wrThreshold)
                {
                    uart0Obj.wrCallback(UART_EVENT_WRITE_THRESHOLD_REACHED, uart0Obj.wrContext);
                }
            }
        }
    }
}

static size_t UART0_WritePendingBytesGet(void)
{
    size_t nPendingTxBytes;
	
	/* Take a snapshot of indices to avoid creation of critical section */
	uint32_t wrOutIndex = uart0Obj.wrOutIndex;
	uint32_t wrInIndex = uart0Obj.wrInIndex;

    if ( wrInIndex >=  wrOutIndex)
    {
        nPendingTxBytes =  wrInIndex -  wrOutIndex;
    }
    else
    {
        nPendingTxBytes =  (UART0_WRITE_BUFFER_SIZE -  wrOutIndex) + wrInIndex;
    }

    return nPendingTxBytes;
}

size_t UART0_WriteCountGet(void)
{
    size_t nPendingTxBytes;    

    nPendingTxBytes = UART0_WritePendingBytesGet();

    return nPendingTxBytes;
}

size_t UART0_Write(uint8_t* pWrBuffer, const size_t size )
{
    size_t nBytesWritten  = 0;

    UART0_TX_INT_DISABLE();

    while (nBytesWritten < size)
    {
        if (UART0_TxPushByte(pWrBuffer[nBytesWritten]) == true)
        {
            nBytesWritten++;
        }
        else
        {
            /* Queue is full, exit the loop */
            break;
        }
    }

    /* Check if any data is pending for transmission */
    if (UART0_WritePendingBytesGet() > 0)
    {
        /* Enable TX interrupt as data is pending for transmission */
        UART0_TX_INT_ENABLE();
    }

    return nBytesWritten;
}

size_t UART0_WriteFreeBufferCountGet(void)
{
    return (UART0_WRITE_BUFFER_SIZE - 1) - UART0_WriteCountGet();
}

size_t UART0_WriteBufferSizeGet(void)
{
    return (UART0_WRITE_BUFFER_SIZE - 1);
}

bool UART0_TransmitComplete(void)
{
    if(UART_SR_TXEMPTY_Msk == (UART0_REGS->UART_SR & UART_SR_TXEMPTY_Msk))
    {
        return true;
    }
	else
	{
		return false;
	}
}

bool UART0_WriteNotificationEnable(bool isEnabled, bool isPersistent)
{
    bool previousStatus = uart0Obj.isWrNotificationEnabled;

    uart0Obj.isWrNotificationEnabled = isEnabled;

    uart0Obj.isWrNotifyPersistently = isPersistent;

    return previousStatus;
}

void UART0_WriteThresholdSet(uint32_t nBytesThreshold)
{
    if (nBytesThreshold > 0)
    {
        uart0Obj.wrThreshold = nBytesThreshold;
    }
}

void UART0_WriteCallbackRegister( UART_RING_BUFFER_CALLBACK callback, uintptr_t context)
{
    uart0Obj.wrCallback = callback;

    uart0Obj.wrContext = context;
}

static void UART0_ISR_RX_Handler( void )
{
    /* Keep reading until there is a character availabe in the RX FIFO */
    while(UART_SR_RXRDY_Msk == (UART0_REGS->UART_SR& UART_SR_RXRDY_Msk))
    {
        if (UART0_RxPushByte( (uint8_t )(UART0_REGS->UART_RHR& UART_RHR_RXCHR_Msk) ) == true)
        {
            UART0_ReadNotificationSend();
        }
        else
        {
            /* UART RX buffer is full */
        }
    }
}

static void UART0_ISR_TX_Handler( void )
{
    uint8_t wrByte;

    /* Keep writing to the TX FIFO as long as there is space */
    while(UART_SR_TXRDY_Msk == (UART0_REGS->UART_SR & UART_SR_TXRDY_Msk))
    {
        if (UART0_TxPullByte(&wrByte) == true)
        {
            UART0_REGS->UART_THR |= wrByte;

            /* Send notification */
            UART0_WriteNotificationSend();
        }
        else
        {
            /* Nothing to transmit. Disable the data register empty interrupt. */
            UART0_TX_INT_DISABLE();
            break;
        }
    }
}

void UART0_InterruptHandler( void )
{
    /* Error status */
    uint32_t errorStatus = (UART0_REGS->UART_SR & (UART_SR_OVRE_Msk | UART_SR_FRAME_Msk | UART_SR_PARE_Msk));

    if(errorStatus != 0)
    {
        /* Client must call UARTx_ErrorGet() function to clear the errors */

        /* Disable Read, Overrun, Parity and Framing error interrupts */

        UART0_REGS->UART_IDR = (UART_IDR_RXRDY_Msk | UART_IDR_FRAME_Msk | UART_IDR_PARE_Msk | UART_IDR_OVRE_Msk);

        /* UART errors are normally associated with the receiver, hence calling
         * receiver callback */
        if( uart0Obj.rdCallback != NULL )
        {
            uart0Obj.rdCallback(UART_EVENT_READ_ERROR, uart0Obj.rdContext);
        }
    }

    /* Receiver status */
    if(UART_SR_RXRDY_Msk == (UART0_REGS->UART_SR & UART_SR_RXRDY_Msk))
    {
        UART0_ISR_RX_Handler();
    }

    /* Transmitter status */
    if(UART_SR_TXRDY_Msk == (UART0_REGS->UART_SR & UART_SR_TXRDY_Msk))
    {
        UART0_ISR_TX_Handler();
    }

}