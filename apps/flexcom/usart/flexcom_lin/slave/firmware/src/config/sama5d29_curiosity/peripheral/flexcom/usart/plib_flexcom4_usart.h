/*******************************************************************************
  FLEXCOM4 USART PLIB

  Company
    Microchip Technology Inc.

  File Name
    plib_flexcom4_usart.h

  Summary
    FLEXCOM4 USART peripheral library interface.

  Description
    This file defines the interface to the FLEXCOM4 USART peripheral library. This
    library provides access to and control of the associated peripheral
    instance.

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

#ifndef PLIB_FLEXCOM4_USART_H // Guards against multiple inclusion
#define PLIB_FLEXCOM4_USART_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "device.h"
#include "plib_flexcom_usart_local.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus // Provide C++ Compatibility
extern "C" {
#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Interface Routines
// *****************************************************************************
// *****************************************************************************
#define FLEXCOM4_USART_FrequencyGet()    (uint32_t)(83000000UL)

/****************************** FLEXCOM4 LIN USART API *********************************/
typedef enum
{
    FLEXCOM_LIN_NACT_PUBLISH = FLEX_US_LINMR_NACT_PUBLISH_Val,
    FLEXCOM_LIN_NACT_SUBSCRIBE = FLEX_US_LINMR_NACT_SUBSCRIBE_Val,
    FLEXCOM_LIN_NACT_IGNORE = FLEX_US_LINMR_NACT_IGNORE_Val,

} FLEXCOM_LIN_NACT;

typedef enum
{
    FLEXCOM_LIN_DATA_LEN_DLC = FLEX_US_LINMR_DLM(0U),
    FLEXCOM_LIN_DATA_LEN_IDENTIFIER = FLEX_US_LINMR_DLM(1U),
} FLEXCOM_LIN_DATA_LEN;

typedef enum
{
    FLEXCOM_LIN_2_0_ENHANCHED_CHECKSUM = FLEX_US_LINMR_CHKTYP(0U) ,
    FLEXCOM_LIN_1_3_CLASSIC_CHECKSUM = FLEX_US_LINMR_CHKTYP(1U) ,
} FLEXCOM_LIN_CHECKSUM_TYPE;

typedef void (* FLEXCOM_LIN_CALLBACK)(uintptr_t context );

typedef struct
{
    FLEXCOM_LIN_CALLBACK idCallback;
    
    uintptr_t idContext;
    
    FLEXCOM_LIN_CALLBACK tranferCallback;
    
    uintptr_t tranferContext;
    
    FLEXCOM_LIN_CALLBACK breakCallback;
    
    uintptr_t breakContext;

} FLEXCOM_LIN_CALLBACK_OBJECT;

/****************************** FLEXCOM4 USART API *********************************/

void FLEXCOM4_USART_Initialize( void );

FLEXCOM_USART_ERROR FLEXCOM4_USART_ErrorGet( void );

bool FLEXCOM4_USART_SerialSetup( FLEXCOM_USART_SERIAL_SETUP* setup, uint32_t srcClkFreq );

size_t FLEXCOM4_USART_Write(uint8_t* pWrBuffer, const size_t size );

size_t FLEXCOM4_USART_Read(uint8_t* pRdBuffer, const size_t size);

size_t FLEXCOM4_USART_WriteFreeBufferCountGet(void);

size_t FLEXCOM4_USART_WriteBufferSizeGet(void);

bool FLEXCOM4_USART_WriteNotificationEnable(bool isEnabled, bool isPersistent);

void FLEXCOM4_USART_WriteThresholdSet(uint32_t nBytesThreshold);

size_t FLEXCOM4_USART_ReadFreeBufferCountGet(void);

size_t FLEXCOM4_USART_ReadBufferSizeGet(void);

bool FLEXCOM4_USART_ReadNotificationEnable(bool isEnabled, bool isPersistent);

void FLEXCOM4_USART_ReadThresholdSet(uint32_t nBytesThreshold);

size_t FLEXCOM4_USART_WriteCountGet(void);

size_t FLEXCOM4_USART_ReadCountGet(void);

void FLEXCOM4_USART_WriteCallbackRegister( FLEXCOM_USART_RING_BUFFER_CALLBACK callback, uintptr_t context);

void FLEXCOM4_USART_ReadCallbackRegister( FLEXCOM_USART_RING_BUFFER_CALLBACK callback, uintptr_t context);

bool FLEXCOM4_USART_TransmitComplete(void);

void FLEXCOM4_LIN_NodeActionSet( FLEXCOM_LIN_NACT action );

bool FLEXCOM4_LIN_IdentifierWrite( uint8_t id);

uint8_t FLEXCOM4_LIN_IdentifierRead( void);

void FLEXCOM4_LIN_ParityEnable(bool parityEnable);

void FLEXCOM4_LIN_ChecksumEnable(bool checksumEnable);

void FLEXCOM4_LIN_ChecksumTypeSet(FLEXCOM_LIN_CHECKSUM_TYPE checksumType);

void FLEXCOM4_LIN_DataLenModeSet(FLEXCOM_LIN_DATA_LEN dataLenMode);

void FLEXCOM4_LIN_ResponseDataLenSet(uint8_t len);

uint8_t FLEXCOM4_LIN_TransferComplete(void);

void FLEXCOM4_LINIdCallbackRegister( FLEXCOM_LIN_CALLBACK callback, uintptr_t context);

void FLEXCOM4_LINTcCallbackRegister( FLEXCOM_LIN_CALLBACK callback, uintptr_t context);

void FLEXCOM4_LINBreakCallbackRegister( FLEXCOM_LIN_CALLBACK callback, uintptr_t context);

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility
    }
#endif
// DOM-IGNORE-END

#endif //PLIB_FLEXCOM4_USART_H
