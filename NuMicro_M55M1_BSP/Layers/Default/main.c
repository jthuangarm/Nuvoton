/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * @brief    Board main function.
 *
 * @copyright SPDX-License-Identifier: Apache-2.0
 * @copyright Copyright (C) 2026 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/

#include "main.h"

#include "RTE_Components.h"
#include  CMSIS_device_header
#include "cmsis_vio.h"

#ifdef    CMSIS_shield_header
#include  CMSIS_shield_header
#endif

#include "NuMicro.h"
#include "hyperram_code.h"
#include "ethosu_driver.h"

/*--------------------------------------------------------------------------*/
static void SYS_Init(void)
{
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init System Clock                                                                                       */
    /*---------------------------------------------------------------------------------------------------------*/

    /* Switch SCLK clock source to APLL0 and Enable APLL0 220MHz clock */
    CLK_SetBusClock(CLK_SCLKSEL_SCLKSEL_APLL0, CLK_APLLCTL_APLLSRC_HXT, FREQ_220MHZ);

    /* Update System Core Clock */
    SystemCoreClockUpdate();

    /* Enable APLL1 192MHz clock */
    CLK_EnableAPLL(CLK_APLLCTL_APLLSRC_HXT, FREQ_192MHZ, CLK_APLL1_SELECT);

    /* Enable FMC0 module clock to keep FMC clock when CPU idle but NPU running */
    CLK_EnableModuleClock(FMC0_MODULE);

    /* Enable the Ethos-U55 clock */
    CLK_EnableModuleClock(NPU0_MODULE);

    /* Enable clock for image sensor (camera) */
    CLK_EnableModuleClock(CCAP0_MODULE);

    /* Select USB clock source as APLL1/2 and USB clock divider as 2 */
    CLK_SetModuleClock(USBD0_MODULE, CLK_USBSEL_USBSEL_APLL1_DIV2, CLK_USBDIV_USBDIV(2));

    /* Debug UART clock setting */
    SetDebugUartCLK();

    /* Enable USB FS */
    /* Enable OTG0 module clock */
    CLK_EnableModuleClock(OTG0_MODULE);

    /* Select USBD */
    SYS->USBPHY = (SYS->USBPHY & ~SYS_USBPHY_USBROLE_Msk) | SYS_USBPHY_OTGPHYEN_Msk ;

    /* Enable USBD module clock */
    CLK_EnableModuleClock(USBD0_MODULE);

    /* Enable USB HS */
    /* Enable HSOTG module clock */
    CLK_EnableModuleClock(HSOTG0_MODULE);

    /* Select HSOTG PHY Reference clock frequency which is from HXT */
    HSOTG_SET_PHY_REF_CLK(HSOTG_PHYCTL_FSEL_24_0M);

    /* Set HSUSB role to HSUSBD */
    SET_HSUSBDROLE();

    /* Enable HSUSB PHY */
    SYS_Enable_HSUSB_PHY();

    /* Enable HSUSBD peripheral clock */
    CLK_EnableModuleClock(HSUSBD0_MODULE);

    /* Enable all GPIO clocks */
    CLK_EnableModuleClock(GPIOA_MODULE);
    CLK_EnableModuleClock(GPIOB_MODULE);
    CLK_EnableModuleClock(GPIOC_MODULE);
    CLK_EnableModuleClock(GPIOD_MODULE);
    CLK_EnableModuleClock(GPIOE_MODULE);
    CLK_EnableModuleClock(GPIOF_MODULE);
    CLK_EnableModuleClock(GPIOG_MODULE);
    CLK_EnableModuleClock(GPIOH_MODULE);
    CLK_EnableModuleClock(GPIOI_MODULE);
    CLK_EnableModuleClock(GPIOJ_MODULE);

    /*---------------------------------------------------------------------------------------------------------*/
    /* Init I/O Multi-function                                                                                 */
    /*---------------------------------------------------------------------------------------------------------*/

    /* Set multi-function pins for UART RXD and TXD */
    SetDebugUartMFP();

    HyperRAM_PinConfig(SPIM0);

    /* USBD multi-function pins for VBUS, D+, D-, and ID pins */
    SET_USB_VBUS_PA12();
    SET_USB_D_MINUS_PA13();
    SET_USB_D_PLUS_PA14();
    SET_USB_OTG_ID_PA15();
}

/*---------------------------------------------------------------------------------------------------------*/
/*  Main Function                                                                                          */
/*---------------------------------------------------------------------------------------------------------*/
int main(void)
{

    /* Unlock protected registers */
    SYS_UnlockReg();

    /* Init System, peripheral clock and multi-function I/O */
    SYS_Init();

    /* Init UART to 115200-8n1 for print message */
    InitDebugUart();

    /* Initialize HyperRAM */
    HyperRAM_Init(SPIM0);

    /* Enter direct-mapped mode to run new applications */
    SPIM_HYPER_EnterDirectMapMode(SPIM0);

    /* Initialize Virtual I/O */
    vioInit();

#if defined(ETHOSU_ARCH)
    /* Initialize Ethos NPU */
    ethos_setup();
#endif

#ifdef CMSIS_shield_header
    shield_setup();
#endif

    /* Lock protected registers */
    SYS_LockReg();

    return (app_main());
}
