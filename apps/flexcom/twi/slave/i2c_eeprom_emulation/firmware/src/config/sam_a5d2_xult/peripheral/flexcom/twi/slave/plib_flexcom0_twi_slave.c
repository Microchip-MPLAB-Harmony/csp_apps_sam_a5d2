/*******************************************************************************
  FLEXCOM TWI Peripheral Library Source File

  Company
    Microchip Technology Inc.

  File Name
    plib_flexcom0_twi_slave.c

  Summary
    FLEXCOM TWI Slave peripheral library interface.

  Description
    This file defines the interface to the FLEXCOM TWI peripheral library. This
    library provides access to and control of the associated peripheral
    instance.

  Remarks:

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
// Included Files
// *****************************************************************************
// *****************************************************************************

#include "plib_flexcom0_twi_slave.h"
#include "interrupts.h"

// *****************************************************************************
// *****************************************************************************
// Global Data
// *****************************************************************************
// *****************************************************************************

volatile static FLEXCOM_TWI_SLAVE_OBJ flexcom0TWIObj;

// *****************************************************************************
// *****************************************************************************
// FLEXCOM0 TWI PLib Interface Routines
// *****************************************************************************
// *****************************************************************************

void FLEXCOM0_TWI_Initialize(void)
{
    /* Set FLEXCOM TWI operating mode */
    FLEXCOM0_REGS->FLEX_MR = FLEX_MR_OPMODE_TWI;

    // Reset the i2c Module
    FLEXCOM0_REGS->FLEX_TWI_CR = FLEX_TWI_CR_SWRST_Msk;

    // Disable the I2C Master/Slave Mode
    FLEXCOM0_REGS->FLEX_TWI_CR = FLEX_TWI_CR_MSDIS_Msk | FLEX_TWI_CR_SVDIS_Msk;

    /* Configure slave address */
    FLEXCOM0_REGS->FLEX_TWI_SMR = FLEX_TWI_SMR_SADR(0x54U);

    /* Clear the Transmit Holding Register and set TXRDY, TXCOMP flags */
    FLEXCOM0_REGS->FLEX_TWI_CR = FLEX_TWI_CR_THRCLR_Msk;

    /* Enable interrupt for Slave Access and End of Slave Access */
    FLEXCOM0_REGS->FLEX_TWI_IER = FLEX_TWI_IER_SVACC_Msk | FLEX_TWI_IER_EOSACC_Msk;

    /* Initialize the TWI PLIB Object */
    flexcom0TWIObj.callback = NULL;
    flexcom0TWIObj.isBusy = false;
    flexcom0TWIObj.isAddrMatchEventNotified = false;
    flexcom0TWIObj.isFirstTxPending = false;

    /* Enable Slave Mode */
    FLEXCOM0_REGS->FLEX_TWI_CR = FLEX_TWI_CR_SVEN_Msk;
}

uint8_t FLEXCOM0_TWI_ReadByte(void)
{
    return (uint8_t)(FLEXCOM0_REGS->FLEX_TWI_RHR & FLEX_TWI_RHR_RXDATA_Msk);
}

void FLEXCOM0_TWI_WriteByte(uint8_t wrByte)
{
    FLEXCOM0_REGS->FLEX_TWI_THR = FLEX_TWI_THR_TXDATA((uint32_t)wrByte);
}

FLEXCOM_TWI_SLAVE_TRANSFER_DIR FLEXCOM0_TWI_TransferDirGet(void)
{
    return ((FLEXCOM0_REGS->FLEX_TWI_SR & FLEX_TWI_SR_SVREAD_Msk) != 0U)? FLEXCOM_TWI_SLAVE_TRANSFER_DIR_READ: FLEXCOM_TWI_SLAVE_TRANSFER_DIR_WRITE;
}

FLEXCOM_TWI_SLAVE_ACK_STATUS FLEXCOM0_TWI_LastByteAckStatusGet(void)
{
    return ((FLEXCOM0_REGS->FLEX_TWI_SR & FLEX_TWI_SR_NACK_Msk) != 0U)? FLEXCOM_TWI_SLAVE_ACK_STATUS_RECEIVED_NAK : FLEXCOM_TWI_SLAVE_ACK_STATUS_RECEIVED_ACK;
}

void FLEXCOM0_TWI_NACKDataPhase(bool isNACKEnable)
{
    if (isNACKEnable == true)
    {
        FLEXCOM0_REGS->FLEX_TWI_SMR = (FLEXCOM0_REGS->FLEX_TWI_SMR & ~FLEX_TWI_SMR_NACKEN_Msk) | FLEX_TWI_SMR_NACKEN_Msk;
    }
    else
    {
        FLEXCOM0_REGS->FLEX_TWI_SMR = (FLEXCOM0_REGS->FLEX_TWI_SMR & ~FLEX_TWI_SMR_NACKEN_Msk);
    }
}


void FLEXCOM0_TWI_CallbackRegister(FLEXCOM_TWI_SLAVE_CALLBACK callback, uintptr_t contextHandle)
{
    flexcom0TWIObj.callback = callback;

    flexcom0TWIObj.context  = contextHandle;
}

bool FLEXCOM0_TWI_IsBusy(void)
{
    return flexcom0TWIObj.isBusy;
}

void __attribute__((used)) FLEXCOM0_InterruptHandler( void )
{
    uint32_t status = FLEXCOM0_REGS->FLEX_TWI_SR;

    if ((status & FLEXCOM0_REGS->FLEX_TWI_IMR) != 0U)
    {
        uintptr_t context = flexcom0TWIObj.context;

        if((status & FLEX_TWI_SR_SVACC_Msk) != 0U)
        {
            if (flexcom0TWIObj.isAddrMatchEventNotified == false)
            {
                FLEXCOM0_REGS->FLEX_TWI_IDR = FLEX_TWI_IDR_SVACC_Msk;

                if (FLEXCOM0_TWI_TransferDirGet() == FLEXCOM_TWI_SLAVE_TRANSFER_DIR_READ)
                {
                    FLEXCOM0_REGS->FLEX_TWI_IER = FLEX_TWI_IER_TXRDY_Msk | FLEX_TWI_IER_TXCOMP_Msk;

                    /* Set flag so that we do not check for NAK from master before transmitting the first byte */
                    flexcom0TWIObj.isFirstTxPending = true;
                }
                else
                {
                    FLEXCOM0_REGS->FLEX_TWI_IER = FLEX_TWI_IER_RXRDY_Msk | FLEX_TWI_IER_TXCOMP_Msk;
                }

                flexcom0TWIObj.isBusy = true;

                if (flexcom0TWIObj.callback != NULL)
                {
                    (void)flexcom0TWIObj.callback(FLEXCOM_TWI_SLAVE_TRANSFER_EVENT_ADDR_MATCH, context);
                }

                flexcom0TWIObj.isAddrMatchEventNotified = true;
            }

            /* I2C Master reads from slave */
            if (FLEXCOM0_TWI_TransferDirGet() == FLEXCOM_TWI_SLAVE_TRANSFER_DIR_READ)
            {
                if ((status & FLEX_TWI_SR_TXRDY_Msk) != 0U)
                {
                    if ((flexcom0TWIObj.isFirstTxPending == true) || ((status & FLEX_TWI_SR_NACK_Msk) == 0U))
                    {
                        if (flexcom0TWIObj.callback != NULL)
                        {
                           (void)flexcom0TWIObj.callback(FLEXCOM_TWI_SLAVE_TRANSFER_EVENT_TX_READY, context);
                        }
                        flexcom0TWIObj.isFirstTxPending = false;
                    }
                    else
                    {
                        /* NAK received. Turn off TXRDY interrupt. */
                        FLEXCOM0_REGS->FLEX_TWI_IDR = FLEX_TWI_IDR_TXRDY_Msk;
                    }
                }
            }
            else
            {
                /* I2C Master writes to slave */
                if ((status & FLEX_TWI_SR_RXRDY_Msk) != 0U)
                {
                    if (flexcom0TWIObj.callback != NULL)
                    {
                       (void)flexcom0TWIObj.callback(FLEXCOM_TWI_SLAVE_TRANSFER_EVENT_RX_READY, context);
                    }
                }
            }
        }
        else if ((status & FLEX_TWI_SR_EOSACC_Msk) != 0U)
        {
            /* Either Repeated Start or Stop condition received */

            flexcom0TWIObj.isAddrMatchEventNotified = false;

            FLEXCOM0_REGS->FLEX_TWI_IDR = FLEX_TWI_IDR_TXRDY_Msk | FLEX_TWI_IDR_RXRDY_Msk;
            FLEXCOM0_REGS->FLEX_TWI_IER = FLEX_TWI_IER_SVACC_Msk;

            if ((status & FLEX_TWI_SR_TXCOMP_Msk) != 0U)
            {
                /* Stop condition received OR start condition with other slave address detected */

                flexcom0TWIObj.isBusy = true;

                if (flexcom0TWIObj.callback != NULL)
                {
                   (void)flexcom0TWIObj.callback(FLEXCOM_TWI_SLAVE_TRANSFER_EVENT_TRANSMISSION_COMPLETE, context);
                }

                FLEXCOM0_REGS->FLEX_TWI_IDR = FLEX_TWI_IDR_TXCOMP_Msk;
            }
        }
        else
        {

        }
    }
}
