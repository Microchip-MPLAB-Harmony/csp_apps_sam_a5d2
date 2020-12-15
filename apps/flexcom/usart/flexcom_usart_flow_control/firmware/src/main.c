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
#include <string.h>
#include "definitions.h"                // SYS function prototypes


// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************
#define LED_On()                        LED_Clear()
#define LED_Off()                       LED_Set()

#define NUM_TX_RX_BYTES                 100

uint8_t txData[NUM_TX_RX_BYTES];
uint8_t rxData[NUM_TX_RX_BYTES];
uint8_t nBytesRead = 0;

void delay(uint32_t count)
{
    uint32_t i;
    uint32_t counter = 0;
    
    for (i = 0; i < count; i++)
    {
        counter += count;
    }
}

int main ( void )
{
    uint8_t i;
    
    /* Initialize all modules */
    SYS_Initialize ( NULL );
    
    /* Fill tx buffer with known values */
    for (i = 0; i < sizeof(txData); i++)
    {
        txData[i] = i + 1;
    }    
    
    /* To demonstrate RTS/CTS lines, transmit with interrupt enabled and receive in non-interrupt mode.
     * Once the RX FIFO is full, the RTS line will be de-asserted and transmitter will stop sending the data.
     * Eventually, when the receiver reads out sufficient number of bytes from the RX FIFO, such that the
     * number of bytes in the FIFO goes below RXFTHRES2, the RTS line is again asserted and transmitter
     * continues sending the data.
    */
    
    /* Transmit in interrupt mode */
    FLEXCOM0_USART_Write(txData, NUM_TX_RX_BYTES);
        
    /* Simulate a slow receiver. Read character by character with delay between each character */
    while (nBytesRead < NUM_TX_RX_BYTES)
    {
        while (FLEXCOM1_USART_ReceiverIsReady() == false);
        
        FLEXCOM1_USART_Read(&rxData[nBytesRead], 1);
        
        nBytesRead += 1;
        
        /* Add some delay to simulate a slow receiver and hence assert RTS line */
        delay(100000);
    }
    
    if (memcmp(txData, rxData, NUM_TX_RX_BYTES) == 0)
    {
        /* Transfer successful */
        LED_On();
    }
    else
    {
        LED_Off();
    }

    /* Execution should not come here during normal operation */

    return ( EXIT_FAILURE );
}


/*******************************************************************************
 End of File
*/

