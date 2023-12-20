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

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include <string.h>                     // For using string functions
#include "definitions.h"                // SYS function prototypes

#define LED_ON()                        LED_BLUE_Set()
#define LED_OFF()                       LED_BLUE_Clear()

#define READ_SIZE                        (10)
#define MINIMUM_BUFFER_SIZE              (32)

#define UART0_TRANSMIT_ADDRESS          (&UART0_REGS->UART_THR)
#define UART0_RECEIVE_ADDRESS           (&UART0_REGS->UART_RHR)

/* While using cache maintenance CMSIS API to handle cache coherency, buffer
 * size must be multiple of cache line (32 bytes) and buffers must be 32 byte
 * aligned*/
static char __attribute__ ((aligned (32))) writeBuffer[] = "**** Cache \
Maintenance with L2CC ****\r\n**** Demo uses L2CC cache maintenance \
API to handle cache coherency ****\r\n**** Type a buffer of 10 characters and \
observe it echo back ****\r\n**** LED toggles on each time buffer is echoed \
****\r\n";
static char __attribute__ ((aligned (32))) readBuffer[MINIMUM_BUFFER_SIZE] = {0};
static char __attribute__ ((aligned (32))) echoBuffer[MINIMUM_BUFFER_SIZE] = {0};

/* Application Status Data */
static char failureMessage[] = "\r\n\r\n**** Data transfer error ****\r\n";
static volatile bool errorStatus = false;
static volatile bool writeStatus = false;
static volatile bool readStatus = false;

void XDMAC_Callback(XDMAC_TRANSFER_EVENT status, uintptr_t context)
{
    if(status == XDMAC_TRANSFER_COMPLETE)
    {
        if(context == 0)
        {
            writeStatus = true;
        }
        else if(context == 1)
        {
            readStatus = true;
        }
    }
    else
    {
        errorStatus = true;
    }
}

// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************

int main ( void )
{
    /* Initialize all modules */
    SYS_Initialize ( NULL );
    LED_OFF();

    /* Register callback functions for both write and read contexts */
    XDMAC0_ChannelCallbackRegister(XDMAC_CHANNEL_0, XDMAC_Callback, 0);
    XDMAC0_ChannelCallbackRegister(XDMAC_CHANNEL_1, XDMAC_Callback, 1);

    /* Clean cache lines having source buffer before submitting a transfer
     * request to XDMAC to load the latest data in the cache to the actual
     * memory */
    DCACHE_CLEAN_BY_ADDR((void*)writeBuffer, sizeof(writeBuffer));
    XDMAC0_ChannelTransfer(XDMAC_CHANNEL_0, writeBuffer, (const void *)UART0_TRANSMIT_ADDRESS, sizeof(writeBuffer));

    while ( true )
    {
        if(errorStatus == true)
        {
            /* Send error message to console.
             * Using USART directly, since DMA is in error condition */
            errorStatus = false;
            UART0_Write(failureMessage, sizeof(failureMessage));
        }
        else if(readStatus == true)
        {
            /* Echo back received buffer and Toggle LED */
            readStatus = false;

            strcpy(echoBuffer, readBuffer);
            strcat(echoBuffer, "\r\n");

            DCACHE_CLEAN_BY_ADDR((void*)echoBuffer, sizeof(echoBuffer));
            
            XDMAC0_ChannelTransfer(XDMAC_CHANNEL_0, echoBuffer, (const void *)UART0_TRANSMIT_ADDRESS, READ_SIZE+strlen("\r\n"));
            LED_BLUE_Toggle();
        }
        else if(writeStatus == true)
        {
            /* Submit buffer to read user data */
            writeStatus = false;
            
            /* Invalidate cache lines having received buffer before using it
             * to load the latest data in the actual memory to the cache */
            DCACHE_INVALIDATE_BY_ADDR((void*)readBuffer, sizeof(readBuffer));
            XDMAC0_ChannelTransfer(XDMAC_CHANNEL_1, (const void *)UART0_RECEIVE_ADDRESS, readBuffer, READ_SIZE);
        }
        else
        {
            /* Repeat the loop */
            ;
        }
    }

    /* Execution should not come here during normal operation */

    return ( EXIT_FAILURE );
}


/*******************************************************************************
 End of File
*/

