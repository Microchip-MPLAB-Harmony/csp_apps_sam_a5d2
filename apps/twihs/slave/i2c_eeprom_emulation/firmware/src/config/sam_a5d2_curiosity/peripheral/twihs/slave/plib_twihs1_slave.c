/*******************************************************************************
  TWIHS Peripheral Library Source File

  Company
    Microchip Technology Inc.

  File Name
    plib_twihs1_slave.c

  Summary
    TWIHS Slave peripheral library interface.

  Description

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

#include "device.h"
#include "plib_twihs1_slave.h"
#include "interrupts.h"

// *****************************************************************************
// *****************************************************************************
// Global Data
// *****************************************************************************
// *****************************************************************************

volatile static TWIHS_SLAVE_OBJ twihs1Obj;

// *****************************************************************************
// *****************************************************************************
// TWIHS1 PLib Interface Routines
// *****************************************************************************
// *****************************************************************************

void TWIHS1_Initialize( void )
{
    /* Reset the TWIHS Module */
    TWIHS1_REGS->TWIHS_CR = TWIHS_CR_SWRST_Msk;

    /* Disable the TWIHS Master/Slave Mode */
    TWIHS1_REGS->TWIHS_CR = TWIHS_CR_MSDIS_Msk | TWIHS_CR_SVDIS_Msk;

    /* Configure slave address */
    TWIHS1_REGS->TWIHS_SMR = TWIHS_SMR_SADR(0x57);

    /* Clear the Transmit Holding Register and set TXRDY, TXCOMP flags */
    TWIHS1_REGS->TWIHS_CR = TWIHS_CR_THRCLR_Msk;

    /* Initialize the TWIHS PLIB Object */
    /* Enable interrupt for Slave Access and End of Slave Access */
    TWIHS1_REGS->TWIHS_IER = TWIHS_IER_SVACC_Msk | TWIHS_IER_EOSACC_Msk;

    twihs1Obj.callback = NULL;
    twihs1Obj.isBusy = false;
    twihs1Obj.isFirstTxPending = false;
    twihs1Obj.isAddrMatchEventNotified = false;

    /* Enable Slave Mode */
    TWIHS1_REGS->TWIHS_CR = TWIHS_CR_SVEN_Msk;
}

uint8_t TWIHS1_ReadByte(void)
{
    return (uint8_t)(TWIHS1_REGS->TWIHS_RHR & TWIHS_RHR_RXDATA_Msk);
}

void TWIHS1_WriteByte(uint8_t wrByte)
{
    TWIHS1_REGS->TWIHS_THR = TWIHS_THR_TXDATA(wrByte);
}

TWIHS_SLAVE_TRANSFER_DIR TWIHS1_TransferDirGet(void)
{
    return ((TWIHS1_REGS->TWIHS_SR & TWIHS_SR_SVREAD_Msk) != 0U)? TWIHS_SLAVE_TRANSFER_DIR_READ: TWIHS_SLAVE_TRANSFER_DIR_WRITE;
}

TWIHS_SLAVE_ACK_STATUS TWIHS1_LastByteAckStatusGet(void)
{
    return ((TWIHS1_REGS->TWIHS_SR & TWIHS_SR_NACK_Msk) != 0U)? TWIHS_SLAVE_ACK_STATUS_RECEIVED_NAK : TWIHS_SLAVE_ACK_STATUS_RECEIVED_ACK;
}

void TWIHS1_NACKDataPhase(bool isNACKEnable)
{
    if (isNACKEnable == true)
    {
        TWIHS1_REGS->TWIHS_SMR = (TWIHS1_REGS->TWIHS_SMR & ~TWIHS_SMR_NACKEN_Msk) | TWIHS_SMR_NACKEN_Msk;
    }
    else
    {
        TWIHS1_REGS->TWIHS_SMR = (TWIHS1_REGS->TWIHS_SMR & ~TWIHS_SMR_NACKEN_Msk);
    }
}


void TWIHS1_CallbackRegister(TWIHS_SLAVE_CALLBACK callback, uintptr_t contextHandle)
{
    twihs1Obj.callback = callback;

    twihs1Obj.context  = contextHandle;
}

bool TWIHS1_IsBusy(void)
{
    return twihs1Obj.isBusy;
}

void __attribute__((used)) TWIHS1_InterruptHandler( void )
{
    uint32_t status = TWIHS1_REGS->TWIHS_SR;
    uintptr_t context = twihs1Obj.context;

    if ((status & TWIHS1_REGS->TWIHS_IMR) != 0U)
    {
        if((status & TWIHS_SR_SVACC_Msk) != 0U)
        {
            if (twihs1Obj.isAddrMatchEventNotified == false)
            {
                TWIHS1_REGS->TWIHS_IDR = TWIHS_IDR_SVACC_Msk;

                if (TWIHS1_TransferDirGet() == TWIHS_SLAVE_TRANSFER_DIR_READ)
                {
                    TWIHS1_REGS->TWIHS_IER = TWIHS_IER_TXRDY_Msk | TWIHS_IER_TXCOMP_Msk;

                    /* Set flag so that we do not check for NAK from master before transmitting the first byte */
                    twihs1Obj.isFirstTxPending = true;
                }
                else
                {
                    TWIHS1_REGS->TWIHS_IER = TWIHS_IER_RXRDY_Msk | TWIHS_IER_TXCOMP_Msk;
                }

                twihs1Obj.isBusy = true;

                if (twihs1Obj.callback != NULL)
                {
                    (void) twihs1Obj.callback(TWIHS_SLAVE_TRANSFER_EVENT_ADDR_MATCH, context);
                }

                twihs1Obj.isAddrMatchEventNotified = true;
            }

            /* I2C Master reads from slave */
            if (TWIHS1_TransferDirGet() == TWIHS_SLAVE_TRANSFER_DIR_READ)
            {
                if ((status & TWIHS_SR_TXRDY_Msk) != 0U)
                {
                    if ((twihs1Obj.isFirstTxPending == true) || ((status & TWIHS_SR_NACK_Msk) == 0U))
                    {
                        if (twihs1Obj.callback != NULL)
                        {
                            (void) twihs1Obj.callback(TWIHS_SLAVE_TRANSFER_EVENT_TX_READY, context);
                        }
                        twihs1Obj.isFirstTxPending = false;
                    }
                    else
                    {
                        /* NAK received. Turn off TXRDY interrupt. */
                        TWIHS1_REGS->TWIHS_IDR = TWIHS_IDR_TXRDY_Msk;
                    }
                }
            }
            else
            {
                /* I2C Master writes to slave */
                if ((status & TWIHS_SR_RXRDY_Msk) != 0U)
                {
                    if (twihs1Obj.callback != NULL)
                    {
                        (void) twihs1Obj.callback(TWIHS_SLAVE_TRANSFER_EVENT_RX_READY, context);
                    }
                }
            }
        }
        else if ((status & TWIHS_SR_EOSACC_Msk) != 0U)
        {
            /* Either Repeated Start or Stop condition received */

            twihs1Obj.isAddrMatchEventNotified = false;

            TWIHS1_REGS->TWIHS_IDR = TWIHS_IDR_TXRDY_Msk | TWIHS_IDR_RXRDY_Msk;
            TWIHS1_REGS->TWIHS_IER = TWIHS_IER_SVACC_Msk;

            if ((status & TWIHS_SR_TXCOMP_Msk) != 0U)
            {
                /* Stop condition received OR start condition with other slave address detected */

                twihs1Obj.isBusy = true;

                if (twihs1Obj.callback != NULL)
                {
                    (void) twihs1Obj.callback(TWIHS_SLAVE_TRANSFER_EVENT_TRANSMISSION_COMPLETE, context);
                }

                TWIHS1_REGS->TWIHS_IDR = TWIHS_IDR_TXCOMP_Msk;
            }
        }
        else
        {
            /* Do Nothing*/
        }
    }
}
