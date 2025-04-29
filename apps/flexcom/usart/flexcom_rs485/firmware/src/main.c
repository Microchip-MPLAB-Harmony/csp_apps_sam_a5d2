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



/* The USART RS485 example demonstrates how to use USART in RS485 mode.

 * Before running, make sure to connect two boards with RS485 lines. 
 * Use the RS485 adapter board (ATRS485-XPRO) for this purpose. 
 * Match each pair of pins on the two boards respectively: A to A, B to B.
 *
 *   - <b>Board 1 and Board 2</b>
 *   -  UART_TX  (XPRO J100 pin14) <->DI to Mikro BUS MOSI
 *   -  UART_RX  (XPRO J100 pin13) <->RO to Mikro BUS MISO
 *   -  UART RTS (XPRO J100 pin 5) <->DE to Mikro BUS SDA
 *   -  3.3v     (XPRO J100 pin 20)      to Mikro BUS 3.3V
 *   -  GND      (XPRO J100 pin 2 or 19) for long distance. Not used in our case.
 *
 *  RS485 XPlained Board 1        RS485 XPlained Board 2
 *        A     <-------------------->  A
 *        B     <-------------------->  B
 *
 * This example connects two boards through the RS485 interface.
 * One board acts as the transmitter, and the other one acts as the receiver.
 * This is defined by the user.
 */

char errorMessage[] = "\r\n**** DBGU error has occurred ****\r\n";

#define BUFFER_SIZE  1024

static volatile bool errorStatus = false;
static volatile bool writeStatus = false;
static volatile bool readStatus = false;

static __ALIGNED(64) uint8_t read_buffer[BUFFER_SIZE];
static __ALIGNED(64) uint8_t write_buffer[BUFFER_SIZE];

/** Transmit buffer. */
uint8_t palette[BUFFER_SIZE]=
"This application provides an example of how to use USART in RS485 mode.\n\r\
The USART features RS485 mode to enable line driver control.\n\r\
While operating in RS485 mode, the USART behaves as though it is in asynchronous\n\r\
or synchronous mode, and configuration of all the parameters is possible.\n\r\
The difference is that the RTS pin is driven high when the transmitter is\n\r\
operating. The behavior of the RTS pin is controlled by the TXEMPTY bit.\n\r";

/* Display main menu. */
static void display_menu( void )
{
    printf("Menu:\n\r");
    printf(" t: transmit the pattern to RS485\n\r");
    printf(" r: receive data from RS485\n\r");
    printf(" m: display menu\r\n");
}

void APP_WriteCallback(uintptr_t context)
{
    writeStatus = true;
}

void APP_ReadCallback(uintptr_t context)
{
    printf("APP_ReadCallback\n\r");
    if(FLEXCOM4_USART_ErrorGet() != FLEXCOM_USART_ERROR_NONE)
    {
        /* ErrorGet clears errors, set error flag to notify console */
        errorStatus = true;
    }
    else
    {
        readStatus = true;
    }
}

/* Dump buffer to DBGU */
static void dump_info(uint8_t *buf, uint32_t size)
{
    uint32_t i = 0 ;

    while ((i < size) && (buf[i] != 0))
    {
        printf("%c", buf[i++]);
    }
}

// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************
int i=0;
int main ( void )
{
    char c = 0;

    /* Initialize all modules */
    SYS_Initialize ( NULL );

        /* Register callback functions and send start message */
    FLEXCOM4_USART_WriteCallbackRegister(APP_WriteCallback, 0);
    FLEXCOM4_USART_ReadCallbackRegister(APP_ReadCallback, 0);

    /* Output example information */
    printf("Usart RS485 Example\n\r");

    /* display main menu*/
    display_menu();

    /* Init transfer buffer */
    memcpy(write_buffer, palette, BUFFER_SIZE);

    while(1)
    {
        /* Check if there is a received character */
        if(UART0_TransmitterIsReady() == true)
        {
            if(UART0_ErrorGet() == UART_ERROR_NONE)
            {
                UART0_Read(&c, 1);
                printf("Key pressed detected: %c\n\r", c);
                switch(c)
                {
                    case 't':
                    case 'T':
                        printf("RS485 transmission...\r\n");
                        FLEXCOM4_USART_Write(write_buffer, sizeof(write_buffer));
                        printf("RS485 transmission completed\r\n");
                    break;

                    case 'r':
                    case 'R':
                        printf("RS485 reception...\r\n");
                        FLEXCOM4_USART_Read(read_buffer, sizeof(read_buffer));
                        while(  readStatus == false)
                        {
                        }
                        readStatus = false;
                        if(errorStatus == true)
                        {
                            /* Send error message to console */
                            errorStatus = false;
                            printf("FLEXCOM USART error occurred\n\r");
                        }
                        dump_info(read_buffer, sizeof(read_buffer));
                        
                        memset(read_buffer, 0, sizeof(read_buffer));
                        printf("RS485 reception completed\r\n");
                        break;

                    case 'm':
                    case 'M':
                        display_menu();
                    break;
                }
            }
            else
            {
                UART0_Write(errorMessage,sizeof(errorMessage));
            }
        }
    }

    /* Execution should not come here during normal operation */

    return ( EXIT_FAILURE );
}


/*******************************************************************************
 End of File
*/

