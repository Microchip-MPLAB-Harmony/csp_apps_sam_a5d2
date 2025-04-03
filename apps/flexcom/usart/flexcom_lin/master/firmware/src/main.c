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

//#define TRACE(fmt, ...)    printf(fmt,##__VA_ARGS__)
#define TRACE(fmt, ...)    

#define LIN_NUM_OF_BYTES_TRANSFER_FROM_HOST_TO_CLIENT           1
#define LIN_NUM_OF_BYTES_TRANSFER_FROM_CLIENT_TO_HOST           1

/* LIN IDs */
#define READ_SWITCH_LED_PID     0x80
#define WRITE_LED_PID           0xC1


typedef enum
{
    APP_STATE_INITIALIZE,
    APP_STATE_SUBSCRIBE,
    APP_STATE_PUBLISH,
    APP_STATE_WAIT_FOR_DATA_TRANSFER_COMPLETE,
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

uint8_t txData[1];
uint8_t txBuffer[10];
uint8_t rxBuffer[10];

uint8_t nTxBytes = 0;
volatile bool LINDataTransferComplete = false;
volatile bool LINIdTransferComplete = false;
volatile bool readNotification  = false;

/* Callback handler that gets called when a LIN identifier transmission is complete. */
void LIN_Id_Tx_Callback_Handler( uintptr_t context)
{
    TRACE("IDTXC\n\r");
    LINIdTransferComplete = true; 
}

/* Callback handler that gets called when a LIN identifier data transmission is complete. */
void LIN_DataTransferComplete_Callback_Handler( uintptr_t context)
{
    TRACE("TCDC\n\r");
    LINDataTransferComplete = true;
}

void Read_Event_Callback_Handler(FLEXCOM_USART_EVENT event, uintptr_t context )
{
    uint32_t nBytesAvailable = 0;

    readNotification = true;
    if (event == FLEXCOM_USART_EVENT_READ_THRESHOLD_REACHED)
    {
        /* The receiver should at least have the threshold number of bytes in the ring buffer. */
        nBytesAvailable = FLEXCOM4_USART_ReadCountGet();
        
        TRACE("R = %d\n\r", (int)nBytesAvailable);
        /* Read the threshold number of bytes here in the rxBuffer. */
        FLEXCOM4_USART_Read((uint8_t*)&rxBuffer, nBytesAvailable);
    }
}

void LIN_MasterTransmit(uint8_t pid, uint8_t* pData, uint8_t nBytes)
{
    nTxBytes = 0;
    
    while (nBytes--)
    {
        txBuffer[nTxBytes++] = *pData++;
    }    
    /* Sets the response data length for LIN communication. */
    FLEXCOM4_LIN_ResponseDataLenSet(LIN_NUM_OF_BYTES_TRANSFER_FROM_HOST_TO_CLIENT);
    
    /* Transmit PID */
    FLEXCOM4_LIN_IdentifierWrite(pid);

    TRACE("T = %d\n\r", (int)nTxBytes);
    /* Transmit DATA + CHECKSUM */ 
    FLEXCOM4_USART_Write(txBuffer, nTxBytes);
}

void LIN_MasterReceive(uint8_t pid)
{
    TRACE("MR\n\r");
    /* To receive data from the host, the master needs to send a PID. */
    FLEXCOM4_LIN_IdentifierWrite(pid);
}

static bool alarmTriggered = false;

/* This function is called after period expires */
void TC0_CH0_TimerInterruptHandler(TC_TIMER_STATUS status, uintptr_t context)
{
    alarmTriggered = true;
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

    printf("LIN master\n\r");
    LED_BLUE_Clear();

    /* Register callback function for CH0 period interrupt */
    TC0_CH0_TimerCallbackRegister(TC0_CH0_TimerInterruptHandler, (uintptr_t)NULL);

    while ( true )
    {
        /* Check the application's current state. */
        switch (appData.state)
        {
            case APP_STATE_INITIALIZE:
                
                LINDataTransferComplete = false;
                readNotification = false;
                LINIdTransferComplete = false;

                /* The LIN_EN (output enable) pin of the LIN-Click board has to be set to the proper state. */
                LIN_EN_Set();

                /* Register a callback for LIN id Transmit events */
                FLEXCOM4_LINIdCallbackRegister(LIN_Id_Tx_Callback_Handler, (uintptr_t)NULL);

                /* Register a callback for LIN data transfer complete events */
                FLEXCOM4_LINTcCallbackRegister(LIN_DataTransferComplete_Callback_Handler, (uintptr_t)NULL);
                
                /* Register a callback for read events */
                FLEXCOM4_USART_ReadCallbackRegister(Read_Event_Callback_Handler, (uintptr_t) NULL);

                /* The LIN response data length is set to 2 (some default value). 
                 * This will be changed as needed in the later part of the code. 
                 * This is only valid if the data length mode is set to DLC mode using the "USART0_LIN_DataLenModeSet()" API. */
                FLEXCOM4_LIN_ResponseDataLenSet(2);

                /* LIN identifier parity is disabled */
                FLEXCOM4_LIN_ParityEnable(false);

                /* LIN hardware checksum is Enabled */
                FLEXCOM4_LIN_ChecksumEnable(true);

                /* LIN frame slot timing is disabled. */
                FLEXCOM4_LIN_FrameSlotEnable(false);

                /* The LIN checksum type is set to 2.0 Enhanced checksum mode. This setting does not have any impact if the checksum feature is disabled. */
                FLEXCOM4_LIN_ChecksumTypeSet(FLEXCOM_LIN_2_0_ENHANCHED_CHECKSUM);
                
                /* LIN data length mode is set to DLC. In this mode, the user has the flexibility to set the response data length using the "USART0_LIN_ResponseDataLenEnable()" API. */
                FLEXCOM4_LIN_DataLenModeSet(FLEXCOM_LIN_DATA_LEN_DLC); 

                /* Enable RX event notifications. */
                FLEXCOM4_USART_ReadNotificationEnable(true, false);
                
                /* Set a threshold value to receive a callback after every "LIN_NUM_OF_BYTES_TRANSFER_FROM_CLIENT_TO_HOST" characters are received. */
                FLEXCOM4_USART_ReadThresholdSet(LIN_NUM_OF_BYTES_TRANSFER_FROM_CLIENT_TO_HOST);

                /* Change the state to "subscribe," as the host is requesting data from the client. */
                appData.state = APP_STATE_SUBSCRIBE;   

                /* Start the timer channel 0*/
                TC0_CH0_TimerStart();
                
                break;
                
            case APP_STATE_SUBSCRIBE:
                
                /* SUBSCRIBE: the node receives the response. */
                /* The host requests data from the client every second. */
                if( alarmTriggered )
                {
                    alarmTriggered = false;
        
                    /* This flag needs to be reset here and will be controlled by the transfer callback handler.*/
                    LINDataTransferComplete = false;

                    /* The LIN response data length is set to "LIN_NUM_OF_BYTES_TRANSFER_FROM_CLIENT_TO_HOST," as the host will expect that many bytes of data from the client. */
                    FLEXCOM4_LIN_ResponseDataLenSet(LIN_NUM_OF_BYTES_TRANSFER_FROM_CLIENT_TO_HOST);

                    /* LIN Host requests client for Switch and LED status. */
                    /* Switch to SUBSCRIBE mode, as the host is requesting data from the client. */
                    FLEXCOM4_LIN_NodeActionSet(FLEX_US_LINMR_NACT_SUBSCRIBE);  

                    /* Transmit client ID. */
                    LIN_MasterReceive(READ_SWITCH_LED_PID);   
                    appData.mode.subscribe = 1;      

                    /* Now we need to wait until the requested data is received from the client. */
                    appData.state = APP_STATE_WAIT_FOR_DATA_TRANSFER_COMPLETE;
                }

                break;

            case APP_STATE_PUBLISH: 

                /* PUBLISH: the node sends the response. */
                TRACE("PUB\n\r");
                /* This flag needs to be reset here and will be controlled by the transfer callback handler later. */
                LINDataTransferComplete = false;
                
                /* Switch to PUBLISH mode, as the host needs to send data to the client. */
                FLEXCOM4_LIN_NodeActionSet(FLEXCOM_LIN_NACT_PUBLISH);
                
                /* Transmit data. */
                LIN_MasterTransmit(WRITE_LED_PID, txData, 1);

                /* Now, we need to wait until the data transfer is complete. */
                appData.state = APP_STATE_WAIT_FOR_DATA_TRANSFER_COMPLETE; 

                break;

            case APP_STATE_PROCESS_DATA:

                TRACE("DATA 0x%X\n\r", rxBuffer[0]);
                /* Check if the requested number of bytes is received by the Ring Buffer. */
                if(readNotification == true)
                {
                    readNotification = false;
                    TRACE("x\n\r");

                    /* Bit position 0 contains the switch status, and bit position 1 contains the LED status. */
                    if ((rxBuffer[0] & 0x01) == 1)
                    {
                            /* As the switch is pressed, ask the LIN slave to toggle its LED. */
                            txData[0] = (((rxBuffer[0] & 0x02) >> 1) ^ 0x01);            
                            
                            /* Change the state to PUBLISH, as the host needs to send data to the client. */
                            appData.state = APP_STATE_PUBLISH;
                            LED_BLUE_Clear();
                            break;
                    }
                    else
                    {
                        LED_BLUE_Set();
                    }

                    /* Change the state to "subscribe," as the host needs to request data from the client again. */
                    appData.state = APP_STATE_SUBSCRIBE; 
                }

                break;

            case APP_STATE_WAIT_FOR_DATA_TRANSFER_COMPLETE:

                /* Wait for the data transfer to complete. When the Host is in PUBLISH mode, the Host needs 
                 * to wait until the specified number of data bytes are transmitted from the HOST to the Client. 
                 * In SUBSCRIBE mode, the Host needs to wait until the requested number of data bytes are transmitted. */
                if(LINDataTransferComplete == true)
                {
                    TRACE("LC 1\n\r");
                    LINDataTransferComplete = false;
                    appData.state = APP_STATE_TRANSFER_COMPLETED;
                }

                break;

            case APP_STATE_TRANSFER_COMPLETED:
                
                if(appData.mode.subscribe == 1)
                {
                    TRACE("Tc 1\n\r");
                    appData.mode.subscribe = 0;
                    appData.state = APP_STATE_PROCESS_DATA;

                }
                else
                {
                    TRACE("Tc 0\n\r");
                    /* change the state to subscribe, as Host needs to request data from Client again*/
                    appData.state = APP_STATE_SUBSCRIBE;
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

