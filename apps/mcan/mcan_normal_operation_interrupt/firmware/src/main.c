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
#include "definitions.h"                // SYS function prototypes

/*************************************************************************************
MCAN - Connect ATA6563 click board to SAMA5D2 Xplained Ultra board as below.
       Connect CANTX0 (PC10) to TX (PIN 13) of ATA6563 click board
       Connect CANRX0 (PC11) to RX (PIN 14) of ATA6563 click board
       Connect GND, 5V and 3.3V to ATA6563 click board
       Connect Microchip CAN Bus Analyzer to PC using USB Male-A to Male Mini-B cable.
       Connect Microchip CAN Bus Analyzer to ATA6563 click board using female to female DB9 serial cable.
       Install Microchip CAN Bus Analyzer software on PC.
MCAN - Normal operation mode, MCAN clock source is 8 MHz and bitrate is 500 Kbps.
*************************************************************************************/

/* LED ON and OFF macros */
#define LED_On()                        LED_Clear()
#define LED_Off()                       LED_Set()
#define WRITE_ID(id)                    (id << 18)
#define READ_ID(id)                     (id >> 18)

uint8_t Mcan0MessageRAM[MCAN0_MESSAGE_RAM_CONFIG_SIZE] __attribute__((aligned (32))) __attribute__((__section__(".region_nocache")));

/* Application's state machine enum */
typedef enum
{
    APP_STATE_MCAN_RECEIVE,
    APP_STATE_MCAN_TRANSMIT,
    APP_STATE_MCAN_IDLE,
    APP_STATE_MCAN_XFER_SUCCESSFUL,
    APP_STATE_MCAN_XFER_ERROR
} APP_STATES;

/* Global variables */
char messageStart[] = "**** MCAN Normal Operation Interrupt Demo ****\r\n\
**** Demo uses interrupt model of MCAN PLIB. ****\r\n\
**** Receive message from CAN Bus and transmit back received message to CAN Bus and UART1 serial port ****\r\n\
**** LED toggles on each time the message is transmitted back ****\r\n";
/* Variable to save application state */
volatile static APP_STATES state = APP_STATE_MCAN_IDLE;
/* Variable to save Tx/Rx transfer status and context */
static uint32_t status = 0;
static uint32_t xferContext = 0;
static uint8_t loop_count = 0;
static uint8_t txFiFo[MCAN0_TX_FIFO_BUFFER_SIZE];
static uint8_t rxFiFo0[MCAN0_RX_FIFO0_SIZE];

// *****************************************************************************
// *****************************************************************************
// Section: Local functions
// *****************************************************************************
// *****************************************************************************
/* Print Rx Message */
static void print_message(uint8_t numberOfMessage, MCAN_RX_BUFFER *rxBuf, uint8_t rxBufLen)
{
    uint8_t length = 0;
    uint8_t msgLength = 0;
    uint32_t id = 0;

    for (uint8_t count = 0; count < numberOfMessage; count++)
    {
        /* Print message to UART1 */
        id = rxBuf->xtd ? rxBuf->id : READ_ID(rxBuf->id);
        msgLength = rxBuf->dlc;
        length = msgLength;
        printf("Message - ID : 0x%x Length : 0x%x ", (unsigned int)id, (unsigned int)msgLength);
        printf("Message : ");
        while(length)
        {
            printf("0x%x ", rxBuf->data[msgLength - length--]);
        }
        printf("\r\n");
        rxBuf += rxBufLen;
    }
}

/* This function will be called by MCAN PLIB when transfer is completed from Tx FIFO */
void APP_MCAN_TxFifoCallback(uintptr_t context)
{
    xferContext = context;

    /* Check MCAN Status */
    status = MCAN0_ErrorGet();

    if (((status & MCAN_PSR_LEC_Msk) == MCAN_ERROR_NONE) || ((status & MCAN_PSR_LEC_Msk) == MCAN_ERROR_LEC_NO_CHANGE))
    {
        switch ((APP_STATES)context)
        {
            case APP_STATE_MCAN_TRANSMIT:
            {
                state = APP_STATE_MCAN_XFER_SUCCESSFUL;
                break;
            }
            default:
                break;
        }
    }
    else
    {
        state = APP_STATE_MCAN_XFER_ERROR;
    }
}

/* This function will be called by MCAN PLIB when Message received in Rx FIFO0 */
void APP_MCAN_RxFifo0Callback(uint8_t numberOfMessage, uintptr_t context)
{
    xferContext = context;

    /* Check MCAN Status */
    status = MCAN0_ErrorGet();

    if (((status & MCAN_PSR_LEC_Msk) == MCAN_ERROR_NONE) || ((status & MCAN_PSR_LEC_Msk) == MCAN_ERROR_LEC_NO_CHANGE))
    {
        switch ((APP_STATES)context)
        {
            case APP_STATE_MCAN_RECEIVE:
            {
                memset(rxFiFo0, 0x00, (numberOfMessage * MCAN0_RX_FIFO0_ELEMENT_SIZE));
                if (MCAN0_MessageReceiveFifo(MCAN_RX_FIFO_0, numberOfMessage, (MCAN_RX_BUFFER *)rxFiFo0) == true)
                {
                    print_message(numberOfMessage, (MCAN_RX_BUFFER *)rxFiFo0, MCAN0_RX_FIFO0_ELEMENT_SIZE);
                    state = APP_STATE_MCAN_TRANSMIT;
                }
                else
                {
                    state = APP_STATE_MCAN_XFER_ERROR;
                }
                break;
            }
            default:
                break;
        }
    }
    else
    {
        state = APP_STATE_MCAN_XFER_ERROR;
    }
}

// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************
int main ( void )
{
    MCAN_TX_BUFFER *txBuffer = NULL;
    MCAN_RX_BUFFER *rxBuf = NULL;

    /* Initialize all modules */
    SYS_Initialize ( NULL );
    LED_Off();

    /* Set Message RAM Configuration */
    MCAN0_MessageRAMConfigSet(Mcan0MessageRAM);

    /* Send start message */
    UART1_Write(&messageStart, sizeof(messageStart));

    MCAN0_RxFifoCallbackRegister(MCAN_RX_FIFO_0, APP_MCAN_RxFifo0Callback, APP_STATE_MCAN_RECEIVE);

    while(1)
    {
        /* Check the application's current state. */
        switch (state)
        {
            case APP_STATE_MCAN_TRANSMIT:
            {
                MCAN0_TxFifoCallbackRegister( APP_MCAN_TxFifoCallback, (uintptr_t)APP_STATE_MCAN_TRANSMIT );
                state = APP_STATE_MCAN_IDLE;
                memset(txFiFo, 0x00, MCAN0_TX_FIFO_BUFFER_SIZE);
                txBuffer = (MCAN_TX_BUFFER *)txFiFo;
                rxBuf = (MCAN_RX_BUFFER *)rxFiFo0;
                txBuffer->id = rxBuf->id;
                txBuffer->dlc = rxBuf->dlc;
                for (loop_count = 0; loop_count < 8; loop_count++){
                    txBuffer->data[loop_count] = rxBuf->data[loop_count];
                }                
                /* Transmit back received Message */
                if (MCAN0_MessageTransmitFifo(1, txBuffer) == false)
                {
                    printf("MCAN0_MessageTransmit request has failed\r\n");
                }
                break;
            }
            case APP_STATE_MCAN_IDLE:
            {
                /* Application can do other task here */
                break;
            }
            case APP_STATE_MCAN_XFER_SUCCESSFUL:
            {
                LED_Toggle();
                state = APP_STATE_MCAN_IDLE;
                break;
            }
            case APP_STATE_MCAN_XFER_ERROR:
            {
                if ((APP_STATES)xferContext == APP_STATE_MCAN_RECEIVE)
                {
                    printf("MCAN Rx Error : 0x%lx\r\n", status);
                }
                else
                {
                    printf("MCAN Tx Error : 0x%lx\r\n", status);
                }
                break;
            }
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

