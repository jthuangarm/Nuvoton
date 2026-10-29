/***********************************************************************/
/*  This file is part of the uVision/ARM development tools             */
/*  Copyright (c) 2006-2023 Keil - An ARM Company.                     */
/*  All rights reserved.                                               */
/***********************************************************************/
/*                                                                     */
/*  FlashDev.C:  Flash Programming Functions adapted for               */
/*               Nuvoton M55M1 Flash using ISP                         */
/*                                                                     */
/***********************************************************************/

#include "..\FlashOS.h"         // FlashOS Structures

#include "M55M1.h"


#if defined(FLASH_APROMNS_BANK1_1M)
#define BANK_OFFSET             0x100000UL
#else
#define BANK_OFFSET             0
#endif

#define CLK_SCLKSEL_SCLKSEL_HIRC            (0x0UL << CLK_SCLKSEL_SCLKSEL_Pos)          /*!< Select System clock source from HIRC    \hideinitializer */
#define CLK_SCLKSEL_SCLKSEL_MIRC            (0x1UL << CLK_SCLKSEL_SCLKSEL_Pos)          /*!< Select System clock source from MIRC    \hideinitializer */
#define CLK_SCLKSEL_SCLKSEL_HIRC48M         (0x2UL << CLK_SCLKSEL_SCLKSEL_Pos)          /*!< Select System clock source from HIRC48M \hideinitializer */
#define CLK_SCLKSEL_SCLKSEL_HXT             (0x3UL << CLK_SCLKSEL_SCLKSEL_Pos)          /*!< Select System clock source from HXT     \hideinitializer */
#define CLK_SCLKSEL_SCLKSEL_APLL0           (0x4UL << CLK_SCLKSEL_SCLKSEL_Pos)          /*!< Select System clock source from APLL0   \hideinitializer */

/*----------------------------------------------------------------------------------------------------------*/
/*  ISPCMD constant definitions                                                                             */
/*----------------------------------------------------------------------------------------------------------*/
#define FMC_ISPCMD_READ         0x00UL          /*!< ISP Command: Read flash word          \hideinitializer */
#define FMC_ISPCMD_READ_UID     0x04UL          /*!< ISP Command: Read Unique ID           \hideinitializer */
#define FMC_ISPCMD_READ_CID     0x0BUL          /*!< ISP Command: Read Company ID          \hideinitializer */
#define FMC_ISPCMD_READ_DID     0x0CUL          /*!< ISP Command: Read Device ID           \hideinitializer */
#define FMC_ISPCMD_READ_CKS     0x0DUL          /*!< ISP Command: Read checksum            \hideinitializer */
#define FMC_ISPCMD_PROGRAM      0x21UL          /*!< ISP Command: Write flash word         \hideinitializer */
#define FMC_ISPCMD_PAGE_ERASE   0x22UL          /*!< ISP Command: Page Erase Flash         \hideinitializer */
#define FMC_ISPCMD_BANK_ERASE   0x23UL          /*!< ISP Command: Erase Flash bank 0 or 1  \hideinitializer */
#define FMC_ISPCMD_CFG_ERASE    0x24UL          /*!< ISP Command: Erase config word        \hideinitializer */
#define FMC_ISPCMD_PROGRAM_MUL  0x27UL          /*!< ISP Command: Multuple word program    \hideinitializer */
#define FMC_ISPCMD_BANK_REMAP   0x2CUL          /*!< ISP Command: Bank Remap               \hideinitializer */
#define FMC_ISPCMD_RUN_CKS      0x2DUL          /*!< ISP Command: Run checksum calculation \hideinitializer */
#define FMC_ISPCMD_VECMAP       0x2EUL          /*!< ISP Command: Vector Page Remap        \hideinitializer */

#define FMC_FLASH_PAGE_SIZE     0x2000UL        /*!< Flash Page Size (8K bytes)            \hideinitializer */

#define __PC()                                                  \
    __extension__({                                             \
        register unsigned int current_pc;                       \
        __ASM volatile("mov %0, pc" : "=r"(current_pc) : : );   \
        current_pc;                                             \
    })    /*!< Current program counter            */


/*
 *  Initialize Flash Programming Functions
 *    Parameter:      adr:  Device Base Address
 *                    clk:  Clock Frequency (Hz)
 *                    fnc:  Function Code (1 - Erase, 2 - Program, 3 - Verify)
 *    Return Value:   0 - OK,  1 - Failed
 */

int Init (unsigned long adr, unsigned long clk, unsigned long fnc)
{
    if (__PC() & NS_OFFSET)
    {
        if (SYS_NS->PDID == 0)
            return (1);

        SYS_NS->REGLCTL  = 0x59;                        // Unlock System Control Registers
        SYS_NS->REGLCTL  = 0x16;
        SYS_NS->REGLCTL  = 0x88;

        if (SYS_NS->REGLCTL != 1)
            return (1);                                 // Not able to unlock

        FMC_NS->ISPCTL = (FMC_ISPCTL_ISPEN_Msk | FMC_ISPCTL_APUEN_Msk | FMC_ISPCTL_ISPFF_Msk);

        if (FMC_NS->ISPCTL == 0)
            return (1);                                 // ISP not enabled

        return (0);
    }
    else
    {
        SYS_S->REGLCTL = 0x59;                          // Unlock System Control Registers
        SYS_S->REGLCTL = 0x16;
        SYS_S->REGLCTL = 0x88;

        if (SYS_S->REGLCTL != 0x01)
            return (1);                                 // Not able to unlock

        CLK_S->SRCCTL |= (CLK_SRCCTL_HIRCEN_Msk | CLK_SRCCTL_HIRC48MEN_Msk);        // Enable HIRC
        CLK_S->FMCCTL  =  CLK_FMCCTL_ISP0CKEN_Msk | CLK_FMCCTL_FMC0CKEN_Msk;        // Enable ISP Clock

        FMC_S->ISPCTL = (FMC_ISPCTL_ISPEN_Msk | FMC_ISPCTL_APUEN_Msk | FMC_ISPCTL_LDUEN_Msk | FMC_ISPCTL_ISPFF_Msk);

        if ((FMC_S->ISPCTL & (FMC_ISPCTL_ISPEN_Msk | FMC_ISPCTL_APUEN_Msk | FMC_ISPCTL_LDUEN_Msk)) != (FMC_ISPCTL_ISPEN_Msk | FMC_ISPCTL_APUEN_Msk | FMC_ISPCTL_LDUEN_Msk))
            return (1);                                 // ISP not enabled

        while ((CLK_S->STATUS & (CLK_STATUS_HIRCSTB_Msk | CLK_STATUS_HIRC48MSTB_Msk)) != (CLK_STATUS_HIRCSTB_Msk | CLK_STATUS_HIRC48MSTB_Msk));

        if ((PMC_S->PLSTS & PMC_PLSTS_PLSTATUS_Msk) > PMC_PLSTS_PLSTATUS_PL1)
        {
            while(PMC_S->PLCTL & PMC_PLCTL_WRBUSY_Msk);

            PMC_S->PLCTL = PMC_PLCTL_PLSEL_PL2;

            while(PMC_S->PLSTS & PMC_PLSTS_PLCBUSY_Msk);
        }

        CLK_S->SCLKSEL = CLK_SCLKSEL_SCLKSEL_HIRC48M;
        CLK_S->SCLKDIV = 0;
    }

    return (0);
}


/*
 *  De-Initialize Flash Programming Functions
 *    Parameter:      fnc:  Function Code (1 - Erase, 2 - Program, 3 - Verify)
 *    Return Value:   0 - OK,  1 - Failed
 */
int UnInit (unsigned long fnc)
{
    FMC_T *tFMC;

    tFMC = ((__PC() & NS_OFFSET) ? FMC_NS : FMC_S);

    while (tFMC->ISPSTS & FMC_ISPSTS_ISPBUSY_Msk);      // Wait until command is finished

    tFMC->ISPCTL = 0;

    return (0);
}


uint32_t Erase(FMC_T *tFMC, uint32_t u32Addr, uint32_t u32Data)
{
    uint32_t u32Reg;

    while (tFMC->ISPSTS & FMC_ISPSTS_ISPBUSY_Msk);      // Wait until command is finished

    tFMC->ISPCTL |= FMC_ISPCTL_ISPFF_Msk;               // Reset Error Flags

    tFMC->ISPCMD  = FMC_ISPCMD_PAGE_ERASE;              // Prepare Command
    tFMC->ISPADDR = u32Addr;                            // Prepare Address
    tFMC->ISPDAT  = u32Data;                            // Prepare Data

    tFMC->ISPTRG  = FMC_ISPTRG_ISPGO_Msk;               // Execute Command
    __ISB();

    while (tFMC->ISPSTS & FMC_ISPSTS_ISPBUSY_Msk);

    u32Reg = tFMC->ISPCTL;

    if (u32Reg & FMC_ISPCTL_ISPFF_Msk)
    {
        tFMC->ISPCTL = u32Reg;
        return (1);
    }

    return (0);                                         // Done
}


/*
 *  Erase Sector in Flash Memory
 *    Parameter:      adr:  Sector Address
 *    Return Value:   0 - OK,  1 - Failed
 */

int EraseSector(unsigned long u32Addr)
{
    FMC_T *tFMC;

    tFMC = ((__PC() & NS_OFFSET) ? FMC_NS : FMC_S);

    u32Addr &= ~NS_OFFSET;
    u32Addr &= ~(FMC_FLASH_PAGE_SIZE - 1);

#if defined(FLASH_APROMNS_BANK1_1M)
    u32Addr += BANK_OFFSET;
#endif

    return Erase(tFMC, u32Addr, 0x00000000);
}

/*
 *  Program Page in Flash Memory
 *    Parameter:      adr:  Page Start Address
 *                    sz:   Page Size
 *                    buf:  Page Data
 *    Return Value:   0 - OK,  1 - Failed
 */

int ProgramPage (unsigned long adr, unsigned long sz, unsigned char *buf)
{
    FMC_T *tFMC;
    uint32_t u32Reg;

    if ((adr & 0x03) != 0)
        return (1);

    tFMC = ((__PC() & NS_OFFSET) ? FMC_NS : FMC_S);

    sz = (sz + 0x03) & ~0x03;                           // Adjust size for Words
    adr &= ~NS_OFFSET;

#if defined(FLASH_APROMNS_BANK1_1M)
    adr += BANK_OFFSET;
#endif

    while (tFMC->ISPSTS & FMC_ISPSTS_ISPBUSY_Msk);      // Wait until command is finished

    tFMC->ISPCTL |= FMC_ISPCTL_ISPFF_Msk;               // Reset Error Flags

    tFMC->ISPCMD = FMC_ISPCMD_PROGRAM;

    while (sz)
    {
        tFMC->ISPADDR = adr;
        tFMC->ISPDAT  = *((uint32_t *)buf);
        tFMC->ISPTRG  = FMC_ISPTRG_ISPGO_Msk;           // Execute Command
        __ISB();

        while (tFMC->ISPSTS & FMC_ISPSTS_ISPBUSY_Msk);

        u32Reg = tFMC->ISPCTL;

        if (u32Reg & FMC_ISPCTL_ISPFF_Msk)
        {
            tFMC->ISPCTL = u32Reg;                      // Reset Error Flags
            return (1);                                 // Failed
        }

        adr += 4;                                       // Go to next Word
        buf += 4;
        sz  -= 4;
    }

    return (0);                                         // Done
}


/*  
 *  Verify Flash Contents
 *    Parameter:      adr:  Start Address
 *                    sz:   Size (in bytes)
 *                    buf:  Data
 *    Return Value:   (adr+sz) - OK, Failed Address
 */

unsigned long Verify (unsigned long adr, unsigned long sz, unsigned char *buf)
{
    FMC_T *tFMC;
    uint32_t u32Reg, u32Offset;

    if ((adr & 0x03) != 0)
        return (adr);

    tFMC = ((__PC() & NS_OFFSET) ? FMC_NS : FMC_S);

    sz = (sz + 0x03) & ~0x03;                           // Adjust size for Words
    u32Offset = adr & NS_OFFSET;
    adr &= ~NS_OFFSET;

#if defined(FLASH_APROMNS_BANK1_1M)
    adr += BANK_OFFSET;
#endif

    while (tFMC->ISPSTS & FMC_ISPSTS_ISPBUSY_Msk);      // Wait until command is finished

    tFMC->ISPCTL |= FMC_ISPCTL_ISPFF_Msk;               // Reset Error Flags

    tFMC->ISPCMD = FMC_ISPCMD_READ;                     // Prepare command

    while (sz)
    {
        tFMC->ISPADDR = adr;                            // Prepare Address
        tFMC->ISPTRG  = FMC_ISPTRG_ISPGO_Msk;           // Execute Command
        __ISB();

        while (tFMC->ISPSTS & FMC_ISPSTS_ISPBUSY_Msk);

        u32Reg = tFMC->ISPCTL;

        if (u32Reg & FMC_ISPCTL_ISPFF_Msk)              // Check for Error
        {
            tFMC->ISPCTL = u32Reg;                      // Reset Error Flags
            break;                                      // Failed
        }

        if (tFMC->ISPDAT != *((uint32_t *)buf))         // Compare Data
        {
            break;
        }

        adr += 4;                                       // Go to next Word
        buf += 4;
        sz  -= 4;
    }

#if defined(FLASH_APROMNS_BANK1_1M)
    adr -= BANK_OFFSET;
#endif

    return (adr | u32Offset);
}
