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
#include <string.h>
#include "definitions.h"                // SYS function prototypes

#define RX_BUFFER_SIZE 256

unsigned char WriteCommand[] = {0x00, 0xB4, 0x00, 0x0A, 0x02, 0x77, 0x55};
unsigned char ReadCommand[] = {0x00, 0xB6, 0x00, 0x0A, 0x02};
unsigned char entire_cfg[] = {0x00, 0xB6, 0x00, 0x00, 0xF0};

uint8_t resp_rcvd[RX_BUFFER_SIZE], resp_bytes;
uint8_t ATR_byteCount=0;

// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************
#define    LED_RED_Off()    LED_RED_Clear()
#define    LED_GREEN_Off()  LED_GREEN_Clear()
#define    LED_BLUE_Off()   LED_BLUE_Clear()
#define    LED_RED_On()    LED_RED_Set()
#define    LED_GREEN_On()  LED_GREEN_Set()
#define    LED_BLUE_On()   LED_BLUE_Set()

int main ( void )
{
    uint8_t index;
     
    /* Initialize all modules */
    SYS_Initialize ( NULL );
    
    LED_RED_Off(); 
    LED_GREEN_Off(); 
    LED_BLUE_Off(); 
    
    printf("\r\n ***********************************************");
    printf("\r\n           ISO 7816 SMART CARD");
    printf("\r\n ***********************************************");
    printf("\r\nPlease insert smart card AT88SC0808C");
    /* Disable card reset */
    FLEXCOM4_ISO7816_Icc_Power_Off();
    
    /* Wait while card detect pin is set */
    while( FLEXCOM4_ISO7816_Card_Detect() != true )
    {
        /* Do Nothing */
    }
    printf("\r\nCard detected");

    /* Enable VCC supply to Card */
    FLEXCOM4_ISO7816_Vcc_Enable();

    /* Enable card Reset */
    FLEXCOM4_ISO7816_Warm_Reset();
    
    memset(resp_rcvd, 0, sizeof(resp_rcvd));
    ATR_byteCount = FLEXCOM4_ISO7816_Data_Read_Atr(resp_rcvd);
    
    if(ATR_byteCount != 0)
    {
        printf("\r\n\r\nATR Response: ");
        for(index=0; index < ATR_byteCount; index++)
        {
            printf("0x%02X ", resp_rcvd[index]);
        }
        FLEXCOM4_ISO7816_Decode_Atr(resp_rcvd, ATR_byteCount);

        printf("\r\n\r\nReading configuration zone data: ");
        for(index=0; index < 5; index++)
        {
            printf("0x%02x ", entire_cfg[index]);
        }
        memset(resp_rcvd, 0, sizeof(resp_rcvd));
        resp_bytes = FLEXCOM4_ISO7816_Xfr_Block_Tpdu(entire_cfg, resp_rcvd, 5);

        printf("\r\nResponse from Card: ");
        for(index=0;index< resp_bytes; index++)
        {
            printf("0x%02x ", resp_rcvd[index]);
        }
        printf("\r\n\r\nWriting 2 bytes of data (0x077, 0x55) at address 0x0A to Card: ");
        for(index=0;index<7; index++)
        {
            printf("0x%02X ", WriteCommand[index]);
        }
        printf("\r\n");
        memset(resp_rcvd, 0, sizeof(resp_rcvd));

        resp_bytes = FLEXCOM4_ISO7816_Xfr_Block_Tpdu(WriteCommand, resp_rcvd, 7);

        printf("\r\nResponse from Card: ");
        for(index=0;index< resp_bytes; index++)
        {
            printf("0x%02X ", resp_rcvd[index]);
        }

        printf("\r\nReading 2 bytes of data from address 0x0A: ");
        for(index=0;index<5; index++)
        {
            printf("0x%02x ", ReadCommand[index]);
        }
        printf("\r\n");

        memset(resp_rcvd, 0, sizeof(resp_rcvd));
        resp_bytes = FLEXCOM4_ISO7816_Xfr_Block_Tpdu(ReadCommand, resp_rcvd, 5);

        printf("\r\nRead Data: ");
        for(index=0;index< 2; index++)
        {
            printf("0x%02x ", resp_rcvd[index]);
        }
        /* Verify the data written and read should be same */
        if ( memcmp(resp_rcvd, &WriteCommand[5], 2) == 0 )
        {
            LED_GREEN_On();
            printf("\r\nTest ok\r\n");
        }
        else
        {
            printf("\r\nTest problem\r\n");
        }
    }
    else
    {
        printf("\r\nSmart card seems to be connected backwards.\r\n");
    }
    /* Disable VCC supply to Card */
    FLEXCOM4_ISO7816_Vcc_Disable();
    
    while ( true )
    {
        /* Do Nothing */
    }

    /* Execution should not come here during normal operation */

    return ( EXIT_FAILURE );
}


/*******************************************************************************
 End of File
*/

