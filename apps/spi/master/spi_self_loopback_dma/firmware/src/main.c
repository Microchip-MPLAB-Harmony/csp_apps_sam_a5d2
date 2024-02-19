/*******************************************************************************
* Copyright (C) 2020 Microchip Technology Inc. and its subsidiaries.
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

/*******************************************************************************
  Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This file contains the "main" function for a project.

  Description:
    This file contains the "main" function for a project.  The
    "main" function calls the "SYS_Initialize" function to initialize the state
    machines of all modules in the system
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include "definitions.h"                // SYS function prototypes
#include <string.h>

// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************
#define TX_BUFFER_SIZE                  256
#define RX_BUFFER_SIZE                  256
#define DMA_CHANNEL_RECEIVE             XDMAC_CHANNEL_0
#define DMA_CHANNEL_TRANSMIT            XDMAC_CHANNEL_1

#define SPI1_RX_ADDR                    &(SPI1_REGS->SPI_RDR)
#define SPI1_TX_ADDR                    &(SPI1_REGS->SPI_TDR)

#define LED_ON()                        LED_GREEN_Set()
#define LED_OFF()                       LED_GREEN_Clear()

CACHE_ALIGN uint8_t txDummyData[32];
uint32_t rxDummyData;
uint32_t txIndex;

volatile bool isTransferComplete = false;
uint32_t txDummyDataSize = 0;
uint32_t rxDummyDataSize = 0;
CACHE_ALIGN uint8_t  transmitBuffer[TX_BUFFER_SIZE];
CACHE_ALIGN uint8_t  receiveBuffer[RX_BUFFER_SIZE];

typedef enum
{
    /* Source address is always fixed */
    XDMAC_SOURCE_ADDRESSING_MODE_FIXED = 0x0,

    /* Source address is incremented after every transfer */
    XDMAC_SOURCE_ADDRESSING_MODE_INCREMENTED = 0x10000,
            
} XDMAC_SOURCE_ADDRESSING_MODE;

typedef enum
{
    /* Destination address is always fixed */
    XDMAC_DESTINATION_ADDRESSING_MODE_FIXED = 0x0,

    /* Destination address is incremented after every transfer */
    XDMAC_DESTINATION_ADDRESSING_MODE_INCREMENTED = 0x40000,
            
} XDMAC_DESTINATION_ADDRESSING_MODE;

void XDMAC_AddressingModeSetup(XDMAC_CHANNEL channel, XDMAC_SOURCE_ADDRESSING_MODE sourceAddrMode, XDMAC_DESTINATION_ADDRESSING_MODE destAddrMode)
{
    uint32_t config;

    config = (uint32_t)XDMAC0_ChannelSettingsGet((XDMAC_CHANNEL)channel);
    config &= ~(0x30000 | 0xC0000);

    config |= (uint32_t)sourceAddrMode | (uint32_t)destAddrMode;

    XDMAC0_ChannelSettingsSet((XDMAC_CHANNEL)channel, (XDMAC_CHANNEL_CONFIG)config);
}

static void APP_DMA_TxCallbackHandler(XDMAC_TRANSFER_EVENT event, uintptr_t context)
{
    if (txDummyDataSize > 0)
    {
        /* Configure DMA channel to transmit (dummy data) from the same location
         * (Source address not incremented) */
        XDMAC_AddressingModeSetup(DMA_CHANNEL_TRANSMIT, XDMAC_SOURCE_ADDRESSING_MODE_FIXED, XDMAC_DESTINATION_ADDRESSING_MODE_FIXED);

        /* Configure the transmit DMA channel */
        XDMAC0_ChannelTransfer(DMA_CHANNEL_TRANSMIT, (const void*)txDummyData, (const void*)SPI1_TX_ADDR, txDummyDataSize);

        txDummyDataSize = 0;
    }   
}

static void APP_DMA_RxCallbackHandler(XDMAC_TRANSFER_EVENT event, uintptr_t context)
{    
    if (rxDummyDataSize > 0)
    {
        /* Configure DMA to receive dummy data */
        XDMAC_AddressingModeSetup(DMA_CHANNEL_RECEIVE, XDMAC_SOURCE_ADDRESSING_MODE_FIXED, XDMAC_DESTINATION_ADDRESSING_MODE_FIXED);

        XDMAC0_ChannelTransfer(DMA_CHANNEL_RECEIVE, (const void*)SPI1_RX_ADDR, (const void *)&rxDummyData, rxDummyDataSize);

        XDMAC0_ChannelTransfer(DMA_CHANNEL_TRANSMIT, (const void *)&((uint8_t*)transmitBuffer)[txIndex], (const void*)SPI1_TX_ADDR, rxDummyDataSize);

        rxDummyDataSize = 0;
    }
    else
    {
        /* Make sure the shift register is empty before de-asserting the CS line */
        while (SPI1_IsBusy());
        
        /* Disable CS */
        SPI_CS_Set();
        
        isTransferComplete = true;
    }
    
}

void spi_read_write_dma(uint8_t* txBuffer, uint32_t txSize, uint8_t* rxBuffer, uint32_t rxSize )
{
    uint32_t size;
    
    DCACHE_CLEAN_BY_ADDR((void*)txBuffer, txSize);    
    DCACHE_INVALIDATE_BY_ADDR((void*)rxBuffer, rxSize);    
    
    /* Enable CS */
    SPI_CS_Clear();
    
    if (rxSize >= txSize)
    {
        /* Dummy data will be sent by the TX DMA */
        txDummyDataSize = (rxSize - txSize);
    }
    else
    {
        /* Dummy data will be received by the RX DMA */
        rxDummyDataSize = (txSize - rxSize);
    }
    
    if (rxSize == 0)
    {
        /* Configure the RX DMA channel - to receive dummy data */
        XDMAC_AddressingModeSetup(DMA_CHANNEL_RECEIVE, XDMAC_SOURCE_ADDRESSING_MODE_FIXED, XDMAC_DESTINATION_ADDRESSING_MODE_FIXED);
        size = rxDummyDataSize;
        rxDummyDataSize = 0;
        XDMAC0_ChannelTransfer(DMA_CHANNEL_RECEIVE, (const void*)SPI1_RX_ADDR, (const void *)&rxDummyData, size);
    }
    else
    {
        /* Configure the RX DMA channel - to receive data in receive buffer */
        XDMAC_AddressingModeSetup(DMA_CHANNEL_RECEIVE, XDMAC_SOURCE_ADDRESSING_MODE_FIXED, XDMAC_DESTINATION_ADDRESSING_MODE_INCREMENTED);
        XDMAC0_ChannelTransfer(DMA_CHANNEL_RECEIVE, (const void*)SPI1_RX_ADDR, (const void *)rxBuffer, rxSize);
    }

    if (txSize == 0)
    {
        /* Configure the TX DMA channel - to send dummy data */
        XDMAC_AddressingModeSetup(DMA_CHANNEL_TRANSMIT, XDMAC_SOURCE_ADDRESSING_MODE_FIXED, XDMAC_DESTINATION_ADDRESSING_MODE_FIXED);
        size = txDummyDataSize;
        txDummyDataSize = 0;
        XDMAC0_ChannelTransfer(DMA_CHANNEL_TRANSMIT, (const void *)txDummyData, (const void*)SPI1_TX_ADDR, size);
    }
    else
    {
        /* Configure the transmit DMA channel - to send data from transmit buffer */
        XDMAC_AddressingModeSetup(DMA_CHANNEL_TRANSMIT, XDMAC_SOURCE_ADDRESSING_MODE_INCREMENTED, XDMAC_DESTINATION_ADDRESSING_MODE_FIXED);

        /* The DMA transfer is split into two for the case where rxSize > 0 && rxSize < txSize */
        if (rxDummyDataSize > 0)
        {
            size = rxSize;
            txIndex = rxSize;
        }
        else
        {
            size = txSize;
        }
        XDMAC0_ChannelTransfer(DMA_CHANNEL_TRANSMIT, (const void *)txBuffer, (const void*)SPI1_TX_ADDR, size);
    }
}

int main ( void )
{
    uint32_t i;
    bool isSuccess = false;
    
    /* Initialize all modules */
    SYS_Initialize ( NULL );
    
    LED_OFF();
    
    /* Fill transmit buffer with known values */
    for (i = 0; i < sizeof(transmitBuffer); i++)
    {
        transmitBuffer[i] = i;
    }
    
    /* Set up the dummy tx data buffer with 0xFF */
    for (i = 0; i < sizeof(txDummyData); i++)
    {
        txDummyData[i] = 0xFF;
    }
    
    /* Push the txDummyData buffer to main memory */
    DCACHE_CLEAN_BY_ADDR((void*)txDummyData, sizeof(txDummyData));   
    
    /* Setup the callbacks */
    XDMAC0_ChannelCallbackRegister(DMA_CHANNEL_TRANSMIT, APP_DMA_TxCallbackHandler, (uintptr_t)NULL);
    XDMAC0_ChannelCallbackRegister(DMA_CHANNEL_RECEIVE, APP_DMA_RxCallbackHandler, (uintptr_t)NULL);
    
    /* Demonstrate - Transmit 10 bytes, Receive 10 bytes */
    spi_read_write_dma(transmitBuffer, 10, receiveBuffer, 10);
    while (isTransferComplete == false);
    isTransferComplete = false;

    if (!memcmp(transmitBuffer, receiveBuffer, 10))
    {
        isSuccess = true;
    }

    /* Demonstrate - Transmit 0 bytes (send dummy 0xFF), Receive 10 bytes */
    if (isSuccess == true)
    {
        spi_read_write_dma(NULL, 0, receiveBuffer, 10);
        while (isTransferComplete == false);
        isTransferComplete = false;

        if (!memcmp(txDummyData, receiveBuffer, 10))
        {
            isSuccess = true;
        }
    }

    /* Demonstrate - Transmit 10 bytes, Receive 5 bytes */
    if (isSuccess == true)
    {
        spi_read_write_dma(transmitBuffer, 10, receiveBuffer, 5);
        while (isTransferComplete == false);
        isTransferComplete = false;

        if (!memcmp(transmitBuffer, receiveBuffer, 5))
        {
            isSuccess = true;
        }
    }

    /* Demonstrate - Transmit 5 bytes, Receive 10 bytes */
    if (isSuccess == true)
    {
        spi_read_write_dma(transmitBuffer, 5, receiveBuffer, 10);
        while (isTransferComplete == false);
        isTransferComplete = false;
        
        /* First 5 received bytes should be same as first 5 bytes in transmitBuffer */
        if (!memcmp(transmitBuffer, receiveBuffer, 5))
        {
            /* Remaining received bytes should be equal to dummy data sent (0xFF) */
            if (!memcmp(txDummyData, &receiveBuffer[5], 5))
            {
                isSuccess = true;
            }
        }
    }

    if (isSuccess == true)
    {
        LED_ON();
    }
    else
    {
        LED_OFF();
    }    

    while ( true )
    {        
        
    }

    /* Execution should not come here during normal operation */

    return ( EXIT_FAILURE );
}


/*******************************************************************************
 End of File
*/

