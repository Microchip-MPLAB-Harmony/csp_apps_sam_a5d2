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

#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include <string.h>
#include "definitions.h"                // SYS function prototypes


#define LIN_NUM_OF_BYTES_FROM_HOST      1
#define LIN_NUM_OF_BYTES_TO_HOST        1

/* LIN IDs */
#define READ_SWITCH_LED_PID             0x80
#define WRITE_LED_PID                   0xC1


typedef enum
{
    APP_STATE_INITIALIZE,
    APP_STATE_SUBSCRIBE,
    APP_STATE_PUBLISH,
    APP_STATE_WAIT_FOR_TRANSFER_COMPLETE,
    APP_STATE_TRANSFER_COMPLETED,
    APP_STATE_PROCESS_DATA,
    APP_STATE_IDLE,

} APP_STATES;

typedef struct
{    
    uint8_t subscribe   :1;
    uint8_t reserved    :7;
} MODE;

typedef struct
{
    volatile APP_STATES  state;
    volatile MODE        mode;
} APP_DATA;

APP_DATA appData;

uint8_t txBuffer[10];
uint8_t rxBuffer[10];
bool ledStatus = false;
volatile bool LINTransferComplete = false;
volatile bool readNotification  = false;
volatile bool LINBreakDetected = false;

//#define TRACE(fmt, ...)    printf(fmt,##__VA_ARGS__)
#define TRACE(fmt, ...)

void LIN_Id_Rcvd_Callback_Handler( uintptr_t context)
{
    /* Read out the PID. */
    rxBuffer[0] = FLEXCOM4_LIN_IdentifierRead();
    switch(rxBuffer[0])
    {            
        case READ_SWITCH_LED_PID:
            /* The client should be in PUBLISH mode now, as the client has to send the data requested by the master. */
            appData.state = APP_STATE_PUBLISH;
            break;

        case WRITE_LED_PID:
            TRACE("W\n\r");
            /* The client should be in SUBSCRIBE mode now, as the client is expecting data from the master. */
            appData.state = APP_STATE_SUBSCRIBE;
            break;
    }
}

void LIN_TransferComplete_Callback_Handler( uintptr_t context)
{
    TRACE("LT\n\r");
    LINTransferComplete = true;
}

void LIN_BreakDetect_Callback_Handler( uintptr_t context)
{
    TRACE("LB\n\r");
    LINBreakDetected = true;
}

void Read_Event_Callback_Handler( FLEXCOM_USART_EVENT event, uintptr_t context )
{
    uint32_t nBytesAvailable = 0;
    TRACE("RECall\n\r");
    readNotification = true;
    if (event == FLEXCOM_USART_EVENT_READ_THRESHOLD_REACHED)
    {
        /* The receiver should at least have the threshold number of bytes in the ring buffer. */
        nBytesAvailable = FLEXCOM4_USART_ReadCountGet();
        TRACE("n = %d\n\r", (int)nBytesAvailable);
        
        /* Read the threshold number of bytes here in the rxBuffer. */
        FLEXCOM4_USART_Read((uint8_t*)&rxBuffer, nBytesAvailable);
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

    printf("LIN slave\n\r");
    LED_BLUE_Clear();
    LED_RED_Clear();
    LED_GREEN_Clear();
    
    while ( true )
    {
        /* Check the application's current state. */
        switch (appData.state)
        {
            case APP_STATE_INITIALIZE:
                
                LINTransferComplete = false;
                readNotification = false;

                /* The LIN_EN (output enable) pin of the LIN-Click board has to be set to the proper state. */
                LIN_EN_Set();

                /* Register a callback for LIN break events. */
                FLEXCOM4_LINBreakCallbackRegister(LIN_BreakDetect_Callback_Handler, (uintptr_t)NULL);

                /* Register a callback for LIN ID received events. */
                FLEXCOM4_LINIdCallbackRegister(LIN_Id_Rcvd_Callback_Handler, (uintptr_t)NULL);

                /* Register a callback for LIN data transfer completion events. */
                FLEXCOM4_LINTcCallbackRegister(LIN_TransferComplete_Callback_Handler, (uintptr_t)NULL);
                
                /* Register a callback for reading events. */
                FLEXCOM4_USART_ReadCallbackRegister(Read_Event_Callback_Handler, (uintptr_t) NULL);
                                
                /* Enable RX event notifications. */
                FLEXCOM4_USART_ReadNotificationEnable(true, false);
                
                /* Set a threshold value to receive a callback after every "LIN_NUM_OF_BYTES_FROM_HOST" characters are received. */
                FLEXCOM4_USART_ReadThresholdSet(LIN_NUM_OF_BYTES_FROM_HOST);
                
                /* The LIN response data length is set to 2 (some default value). This will be changed as needed in the later part of the code. 
                 * This is only valid if the data length mode is set to DLC mode using the "FLEXCOM4_LIN_DataLenModeSet()" API. */
                FLEXCOM4_LIN_ResponseDataLenSet(2);
                
                /* LIN identifier parity is disabled. */
                FLEXCOM4_LIN_ParityEnable(false);
                
                /* The LIN hardware checksum is enabled. */
                FLEXCOM4_LIN_ChecksumEnable(true);
                
                /* The LIN checksum type is set to 2.0 Enhanced checksum mode. This setting does not have any impact if the checksum feature is disabled. */
                FLEXCOM4_LIN_ChecksumTypeSet(FLEXCOM_LIN_2_0_ENHANCHED_CHECKSUM);
                
                /* The LIN data length mode is set to DLC. In this mode, the user has the flexibility to set the response data length using the "FLEXCOM4_LIN_ResponseDataLenEnable()" API. */
                FLEXCOM4_LIN_DataLenModeSet(FLEXCOM_LIN_DATA_LEN_DLC); 
                
                /* Wait for instructions from the LIN master. */
                appData.state = APP_STATE_IDLE;   
                
                break;
                
            case APP_STATE_PUBLISH: 
                
                /* PUBLISH: the node sends the response. */
                /* The LIN Host has requested the switch and LED status. Switch to PUBLISH mode. */
                FLEXCOM4_LIN_NodeActionSet(FLEXCOM_LIN_NACT_PUBLISH);
                FLEXCOM4_LIN_ResponseDataLenSet(LIN_NUM_OF_BYTES_TO_HOST);

                /* Requested switch and LED status. */
                txBuffer[0] = ((ledStatus << 1) | (SWITCH_Get()));
                TRACE("PUB %d\n\r", (int)txBuffer[0]);

                FLEXCOM4_USART_Write(txBuffer, LIN_NUM_OF_BYTES_TO_HOST);

                /* Wait for the current data transfer to complete. In this case, the client is sending data to the host, so you need to wait until this transfer is completed. */
                appData.state = APP_STATE_WAIT_FOR_TRANSFER_COMPLETE; 
                break;

            case APP_STATE_SUBSCRIBE:

                /* SUBSCRIBE: the node receives the response. */
                /* Waiting for data from host. Switch to subscribe mode. */
                FLEXCOM4_LIN_NodeActionSet(FLEXCOM_LIN_NACT_SUBSCRIBE);
                 
                /* The response data length needs to be set to "LIN_NUM_OF_BYTES_FROM_HOST." */
                FLEXCOM4_LIN_ResponseDataLenSet(LIN_NUM_OF_BYTES_FROM_HOST);
                
                appData.mode.subscribe = 1;
                
                /* Wait for the current data transfer to complete. In this case, the host is sending data to the client, so you need to wait until the expected data is received. */
                appData.state = APP_STATE_WAIT_FOR_TRANSFER_COMPLETE;

                break;

            case APP_STATE_PROCESS_DATA:

                TRACE("DATA ");
                /* Check if the receiver has received the threshold number of bytes in the ring buffer. */
                if(readNotification == true)
                {
                    readNotification = false;

                    TRACE(" %d\n\r", (int)rxBuffer[0]);
                    /* Check the LED state requested by the LIN master. */
                    if (rxBuffer[0] & 0x01)
                    {
                        /* Turn on the LED. */
                        TRACE("LED 0\n\r");
                        LED_BLUE_Clear();
                        ledStatus = 1;
                    }
                    else
                    {
                        /* Turn off the LED. */
                        TRACE("LED 1\n\r");
                        LED_BLUE_Set();
                        ledStatus = 0;
                    }

                    /* Wait for instructions from the LIN master. */
                    appData.state = APP_STATE_IDLE; 
                }

                break;

            case APP_STATE_WAIT_FOR_TRANSFER_COMPLETE:

                if(LINTransferComplete == true)
                {
                    LINTransferComplete = false;
                    appData.state = APP_STATE_TRANSFER_COMPLETED;
                }

                break;

            case APP_STATE_TRANSFER_COMPLETED:
                
                if(appData.mode.subscribe == 1)
                {
                    TRACE("subs 1\n\r");
                    appData.mode.subscribe = 0;
                    appData.state = APP_STATE_PROCESS_DATA;

                }
                else
                {
                    TRACE("subs 0\n\r");
                    /* Wait for instructions from LIN master */
                    appData.state = APP_STATE_IDLE;
                }

                break;
            
            case APP_STATE_IDLE: 

                break;

            default:
                break;
        }
    }

    /* Execution should not come here during normal operation */

    return ( EXIT_FAILURE );
}


/*******************************************************************************
 End of File
*/

