/*******************************************************************************
* Copyright (C) 2019 Microchip Technology Inc. and its subsidiaries.
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

#include "device.h"
#include "plib_clk.h"







/*********************************************************************************
Initialize Generic clock
*********************************************************************************/


/*********************************************************************************
Initialize Programmable Clock (PCKx)
*********************************************************************************/

static void CLK_ProgrammableClockInitialize(void)
{
    /* Disable selected programmable clock  */
    PMC_REGS->PMC_SCDR = PMC_SCDR_PCK1_Msk;

    /* Configure selected programmable clock    */
    PMC_REGS->PMC_PCK[1]= PMC_PCK_CSS_MAIN_CLK | PMC_PCK_PRES(0U);

    /* Enable selected programmable clock   */
    PMC_REGS->PMC_SCER = PMC_SCER_PCK1_Msk;

    /* Wait for clock to be ready   */
    while((PMC_REGS->PMC_SR & (PMC_SR_PCKRDY1_Msk) ) != (PMC_SR_PCKRDY1_Msk))
    {
        /* Wait for PCKRDY */
    }
}


/*********************************************************************************
Initialize Peripheral clock
*********************************************************************************/

static void CLK_PeripheralClockInitialize(void)
{
    /* Enable clock for the selected peripherals, since the rom boot will turn on
     * certain clocks turn off all clocks not expressly enabled */
    PMC_REGS->PMC_PCER0=0x1042000U;
    PMC_REGS->PMC_PCDR0=~0x1042000U;
    PMC_REGS->PMC_PCER1=0x0U;
    PMC_REGS->PMC_PCDR1=~0x0U;
}




/*********************************************************************************
Clock Initialize
*********************************************************************************/

void CLK_Initialize( void )
{
    /* Initialize Programmable Clock */
    CLK_ProgrammableClockInitialize();

    /* Initialize Peripheral Clock */
    CLK_PeripheralClockInitialize();

}

