/***********************************************************************/
/*  This file is part of the uVision/ARM development tools             */
/*  Copyright (c) 2006-2017 Keil - An ARM Company.                     */
/*  All rights reserved.                                               */
/***********************************************************************/
/*                                                                     */
/*  FlashDev.C:  Flash Programming Functions adapted for               */
/*               Nuvoton M55M1 External SPI Flash using SPIM           */
/*                                                                     */
/***********************************************************************/

#include "..\..\FlashOS.h"      // FlashOS Structures

#include <string.h>
#include "M55M1.h"


#define CLK_SCLKSEL_SCLKSEL_HIRC            (0x0UL << CLK_SCLKSEL_SCLKSEL_Pos)      /*!< Select System clock source from HIRC    \hideinitializer */
#define CLK_SCLKSEL_SCLKSEL_MIRC            (0x1UL << CLK_SCLKSEL_SCLKSEL_Pos)      /*!< Select System clock source from MIRC    \hideinitializer */
#define CLK_SCLKSEL_SCLKSEL_HIRC48M         (0x2UL << CLK_SCLKSEL_SCLKSEL_Pos)      /*!< Select System clock source from HIRC48M \hideinitializer */
#define CLK_SCLKSEL_SCLKSEL_HXT             (0x3UL << CLK_SCLKSEL_SCLKSEL_Pos)      /*!< Select System clock source from HXT     \hideinitializer */
#define CLK_SCLKSEL_SCLKSEL_APLL0           (0x4UL << CLK_SCLKSEL_SCLKSEL_Pos)      /*!< Select System clock source from APLL0   \hideinitializer */

#define CLK_APLLCTL_APLLSRC_HXT             0x00000000UL    /*!< For APLL clock source is HXT.       4MHz < FIN/NR < 8MHz \hideinitializer */
#define CLK_APLLCTL_APLLSRC_HXT_DIV2        0x00000001UL    /*!< For APLL clock source is HXT/2.     4MHz < FIN/NR < 8MHz \hideinitializer */
#define CLK_APLLCTL_APLLSRC_HIRC            0x00000002UL    /*!< For APLL clock source is HIRC.      4MHz < FIN/NR < 8MHz \hideinitializer */
#define CLK_APLLCTL_APLLSRC_HIRC48_DIV4     0x00000003UL    /*!< For APLL clock source is HIRC48/4.  4MHz < FIN/NR < 8MHz \hideinitializer */


#define SPIFLASH_PAGE_SIZE                  256


void SPIM_IO_Write(uint8_t u8Data)
{
    SPIM0_S->TX[0] = u8Data;

    SPIM0_S->CTL1 |= SPIM_CTL1_SPIMEN_Msk;

    while (SPIM0_S->CTL1 & SPIM_CTL1_SPIMEN_Msk);
}

uint8_t SPIM_IO_Read(void)
{
    SPIM0_S->CTL1 |= SPIM_CTL1_SPIMEN_Msk;

    while (SPIM0_S->CTL1 & SPIM_CTL1_SPIMEN_Msk);

    return (SPIM0_S->RX[0] & 0xFF);
}

void SPIM_DMA_Write(uint32_t u32Addr, uint32_t u32Size, uint8_t *pu8Buf)
{
    SPIM0_S->CTL0 = (SPIM0_S->CTL0 & ~SPIM_CTL0_OPMODE_Msk) | (SPIM_CTL0_OPMODE_PAGEWRITE << SPIM_CTL0_OPMODE_Pos);

    SPIM0_S->CMDCODE = CMD_NORMAL_PAGE_PROGRAM;

    SPIM0_S->SRAMADDR = (uint32_t)pu8Buf;           // SRAM Memory Address

    SPIM0_S->DMACNT = u32Size;                      // DMA Transfer Byte Count

    SPIM0_S->FADDR = u32Addr;                       // SPI Flash Address

    SPIM0_S->CTL1 |= SPIM_CTL1_SPIMEN_Msk;          // Start Transfer

    while (SPIM0_S->CTL1 & SPIM_CTL1_SPIMEN_Msk);   // Wait until SPI transfer is finished
}

void SPIM_DMA_Read(uint32_t u32Addr, uint32_t u32Size, uint8_t *pu8Buf)
{
    SPIM0_S->CTL0 = (SPIM0_S->CTL0 & ~SPIM_CTL0_OPMODE_Msk) | (SPIM_CTL0_OPMODE_PAGEREAD << SPIM_CTL0_OPMODE_Pos);

    SPIM0_S->CMDCODE = CMD_DMA_NORMAL_READ;

    SPIM0_S->SRAMADDR = (uint32_t)pu8Buf;           // SRAM Memory Address

    SPIM0_S->DMACNT = u32Size;                      // DMA Transfer Byte Count

    SPIM0_S->FADDR = u32Addr;                       // SPI Flash Address

    SPIM0_S->CTL1 |= SPIM_CTL1_SPIMEN_Msk;          // Start Transfer

    while (SPIM0_S->CTL1 & SPIM_CTL1_SPIMEN_Msk);   // Wait until SPI transfer is finished
}

uint32_t SPIFLASH_ReadID(void)
{
    uint8_t u8ID0, u8ID1, u8ID2;

    // CS is   active (Low)
    SPIM0_S->CTL1 = (SPIM0_S->CTL1 & (~SPIM_CTL1_SS_Msk)) | (0UL << SPIM_CTL1_SS_Pos);

    SPIM0_S->CTL0 = (SPIM0_S->CTL0 & ~(SPIM_CTL0_OPMODE_Msk | SPIM_CTL0_QDIODIR_Msk)) | ((SPIM_CTL0_OPMODE_IO << SPIM_CTL0_OPMODE_Pos) | (1UL << SPIM_CTL0_QDIODIR_Pos));

    // Send Read ID instruction
    SPIM_IO_Write(OPCODE_RDID);

    SPIM0_S->CTL0 = (SPIM0_S->CTL0 & ~(SPIM_CTL0_OPMODE_Msk | SPIM_CTL0_QDIODIR_Msk)) | ((SPIM_CTL0_OPMODE_IO << SPIM_CTL0_OPMODE_Pos) | (0UL << SPIM_CTL0_QDIODIR_Pos));

    // Read a byte from the FLASH
    u8ID2 = SPIM_IO_Read();
    u8ID1 = SPIM_IO_Read();
    u8ID0 = SPIM_IO_Read();

    // CS is inactive (High)
    SPIM0_S->CTL1 = (SPIM0_S->CTL1 & (~SPIM_CTL1_SS_Msk)) | (1UL << SPIM_CTL1_SS_Pos);

    return ((u8ID2 << 16) | (u8ID1 << 8) | u8ID0);
}

uint8_t SPIFLASH_CheckStatus(uint8_t u8Flag, uint8_t u8Value)
{
    uint8_t u8Status;

    // CS is active (Low)
    SPIM0_S->CTL1 = (SPIM0_S->CTL1 & (~SPIM_CTL1_SS_Msk)) | (0UL << SPIM_CTL1_SS_Pos);

    SPIM0_S->CTL0 = (SPIM0_S->CTL0 & ~(SPIM_CTL0_OPMODE_Msk | SPIM_CTL0_QDIODIR_Msk)) | ((SPIM_CTL0_OPMODE_IO << SPIM_CTL0_OPMODE_Pos) | (1UL << SPIM_CTL0_QDIODIR_Pos));

    // Send read status register
    SPIM_IO_Write(OPCODE_RDSR);

    SPIM0_S->CTL0 = (SPIM0_S->CTL0 & ~(SPIM_CTL0_OPMODE_Msk | SPIM_CTL0_QDIODIR_Msk)) | ((SPIM_CTL0_OPMODE_IO << SPIM_CTL0_OPMODE_Pos) | (0UL << SPIM_CTL0_QDIODIR_Pos));

    do
    {
        u8Status = SPIM_IO_Read();                  // Read status
    }
    while ((u8Status & u8Flag) != u8Value);         // Write in progress

    // CS is inactive (High)
    SPIM0_S->CTL1 = (SPIM0_S->CTL1 & (~SPIM_CTL1_SS_Msk)) | (1UL << SPIM_CTL1_SS_Pos);

    return u8Status;
}

uint8_t SPIFLASH_WriteEnable(void)
{
    uint8_t u8Status;

    // CS is active (Low)
    SPIM0_S->CTL1 = (SPIM0_S->CTL1 & (~SPIM_CTL1_SS_Msk)) | (0UL << SPIM_CTL1_SS_Pos);

    SPIM0_S->CTL0 = (SPIM0_S->CTL0 & ~(SPIM_CTL0_OPMODE_Msk | SPIM_CTL0_QDIODIR_Msk)) | ((SPIM_CTL0_OPMODE_IO << SPIM_CTL0_OPMODE_Pos) | (1UL << SPIM_CTL0_QDIODIR_Pos));

    // Send write enable
    SPIM_IO_Write(OPCODE_WREN);

    // CS is inactive (High)
    SPIM0_S->CTL1 = (SPIM0_S->CTL1 & (~SPIM_CTL1_SS_Msk)) | (1UL << SPIM_CTL1_SS_Pos);

    u8Status = SPIFLASH_CheckStatus((SR_WEL | SR_WIP), SR_WEL);

    return u8Status;
}

uint8_t SPIFLASH_PageProgram(uint32_t u32Addr, uint32_t u32Size, uint8_t *pu8Buf)
{
    uint8_t u8Status;

    u8Status = SPIFLASH_WriteEnable();

    if ((u8Status & SR_WEL) == 0)
        return (1);

    // CS is active (Low)
    SPIM0_S->CTL1 = (SPIM0_S->CTL1 & (~SPIM_CTL1_SS_Msk)) | (0UL << SPIM_CTL1_SS_Pos);

    SPIM_DMA_Write(u32Addr, u32Size, pu8Buf);

    // CS is inactive (High)
    SPIM0_S->CTL1 = (SPIM0_S->CTL1 & (~SPIM_CTL1_SS_Msk)) | (1UL << SPIM_CTL1_SS_Pos);

    u8Status = SPIFLASH_CheckStatus(SR_WIP, 0);

    if (u8Status != 0)
        return (1);

    return (0);
}

uint8_t SPIFLASH_ReadData(uint32_t u32Addr, uint32_t u32Size, uint8_t *pu8Buf)
{
    // CS is active (Low)
    SPIM0_S->CTL1 = (SPIM0_S->CTL1 & (~SPIM_CTL1_SS_Msk)) | (0UL << SPIM_CTL1_SS_Pos);

    SPIM_DMA_Read(u32Addr, u32Size, pu8Buf);

    // CS is inactive (High)
    SPIM0_S->CTL1 = (SPIM0_S->CTL1 & (~SPIM_CTL1_SS_Msk)) | (1UL << SPIM_CTL1_SS_Pos);

    return (0);
}


/*
 *  Initialize Flash Programming Functions
 *    Parameter:      adr:  Device Base Address
 *                    clk:  Clock Frequency (Hz)
 *                    fnc:  Function Code (1 - Erase, 2 - Program, 3 - Verify)
 *    Return Value:   0 - OK,  1 - Failed
 */

int Init (unsigned long adr, unsigned long clk, unsigned long fnc)
{
    uint32_t u32JEDEC_ID;

    SYS->REGLCTL = 0x59;                            // Unlock System Control Registers
    SYS->REGLCTL = 0x16;
    SYS->REGLCTL = 0x88;

    if (SYS->REGLCTL != 0x01)
        return (1);                                 // Not able to unlock

    if ((CLK->STATUS & CLK_STATUS_HIRCSTB_Msk) == 0)
    {
        CLK->SRCCTL |= CLK_SRCCTL_HIRCEN_Msk;           // Enable HIRC

        while (!(CLK->STATUS & CLK_STATUS_HIRCSTB_Msk));
    }

    if ((PMC->PLSTS & PMC_PLSTS_PLSTATUS_Msk) > PMC_PLSTS_PLSTATUS_PL1)
    {
        while(PMC->PLCTL & PMC_PLCTL_WRBUSY_Msk);

        PMC->PLCTL = PMC_PLCTL_PLSEL_PL1;

        while(PMC->PLSTS & PMC_PLSTS_PLCBUSY_Msk);
    }

    if ((CLK->SCLKSEL & CLK_SCLKSEL_SCLKSEL_Msk) == CLK_SCLKSEL_SCLKSEL_APLL0)
    {
        CLK->SCLKSEL = CLK_SCLKSEL_SCLKSEL_HIRC;
    }

    CLK->SRCCTL &= ~CLK_SRCCTL_APLL0EN_Msk;

    CLK->APLL0CTL = 0x10008416;     // 96MHz
    CLK->APLL0SEL = CLK_APLLCTL_APLLSRC_HIRC;

    CLK->SRCCTL |= CLK_SRCCTL_APLL0EN_Msk;
    while (!(CLK->STATUS & CLK_STATUS_APLL0STB_Msk));

    CLK->SCLKSEL = CLK_SCLKSEL_SCLKSEL_APLL0;
    CLK->SCLKDIV = 0;

    CLK->SPIMCTL = CLK_SPIMCTL_SPIM0CKEN_Msk;
    while (!(CLK->SPIMCTL & CLK_SPIMCTL_SPIM0CKEN_Msk));

    SYS->SPIMRST = SYS_SPIMRST_SPIM0RST_Msk;
    SYS->SPIMRST = 0;

    SPIM0_S->CTL0 = (SPIM_CTL0_SPI_RSTN_Msk | (SPIM_CTL0_OPMODE_IO << SPIM_CTL0_OPMODE_Pos) | (SPIM_CTL0_BITMODE_SING << SPIM_CTL0_BITMODE_Pos) | (0x1UL << SPIM_CTL0_QDIODIR_Pos) | (SPIM_CTL0_BURSTNUM_1 << SPIM_CTL0_BURSTNUM_Pos) | ((SPIM_CTL0_DWIDTH_8-1) << SPIM_CTL0_DWIDTH_Pos) | (0x00UL << SPIM_CTL0_DEVMODE_Pos) | SPIM_CTL0_CIPHOFF_Msk);

    SPIM0_S->CTL1 = (SPIM0_S->CTL1 & ~(SPIM_CTL1_DIVIDER_Msk | SPIM_CTL1_SSACTPOL_Msk)) | ((0x2UL << SPIM_CTL1_DIVIDER_Pos) | SPIM_CTL1_SS_Msk | SPIM_CTL1_CACHEOFF_Msk);

    SPIM0_S->DLL0 = (SPIM0_S->DLL0 & ~(SPIM_DLL0_DLLDIVER_Msk | SPIM_DLL0_DLLOVRST_Msk | SPIM_DLL0_DLLOLDO_Msk)) | ((0x03UL << SPIM_DLL0_DLLDIVER_Pos) | (0x01UL << SPIM_DLL0_DLLOVRST_Pos) | (0x01UL << SPIM_DLL0_DLLOLDO_Pos));

    while (SPIM0_S->DLL0 & SPIM_DLL0_DLLOVRST_Msk);

    while ((SPIM0_S->DLL0 & (SPIM_DLL0_DLLREADY_Msk | SPIM_DLL0_DLLLOCK_Msk | SPIM_DLL0_DLLCLKON_Msk)) != (SPIM_DLL0_DLLREADY_Msk | SPIM_DLL0_DLLLOCK_Msk | SPIM_DLL0_DLLCLKON_Msk));

    SPIM0_S->CTL2 = (SPIM0_S->CTL2 & ~SPIM_CTL2_DC_DMAR_Msk) | (0UL << SPIM_CTL2_DC_DMAR_Pos);

    SPIM0_S->PHDMAW = 0x00000301;
    SPIM0_S->PHDMAR = 0x00000301;

    SYS->GPH_MFP3 = (SYS->GPH_MFP3 & ~(SYS_GPH_MFP3_PH13MFP_Msk)) | (0x0CUL << SYS_GPH_MFP3_PH13MFP_Pos);
    SYS->GPJ_MFP0 = (SYS->GPJ_MFP0 & ~(SYS_GPJ_MFP0_PJ3MFP_Msk)) | (0x0CUL << SYS_GPJ_MFP0_PJ3MFP_Pos);
    SYS->GPJ_MFP1 = (SYS->GPJ_MFP1 & ~(SYS_GPJ_MFP1_PJ4MFP_Msk | SYS_GPJ_MFP1_PJ5MFP_Msk | SYS_GPJ_MFP1_PJ6MFP_Msk | SYS_GPJ_MFP1_PJ7MFP_Msk)) | ((0x0CUL << SYS_GPJ_MFP1_PJ4MFP_Pos) | (0x0CUL << SYS_GPJ_MFP1_PJ5MFP_Pos) | (0x0CUL << SYS_GPJ_MFP1_PJ6MFP_Pos) | (0x0CUL << SYS_GPJ_MFP1_PJ7MFP_Pos));

    u32JEDEC_ID = SPIFLASH_ReadID();

    // Winbond, EON, ...
    if ((u32JEDEC_ID == 0x000000) || (u32JEDEC_ID == 0xFFFFFF))
        return (1);

    return (0);
}


/*
 *  De-Initialize Flash Programming Functions
 *    Parameter:      fnc:  Function Code (1 - Erase, 2 - Program, 3 - Verify)
 *    Return Value:   0 - OK,  1 - Failed
 */
int UnInit (unsigned long fnc)
{
    SPIM0_S->CTL1 |= SPIM_CTL1_CDINVAL_Msk;

    SYS->SPIMRST = SYS_SPIMRST_SPIM0RST_Msk;
    SYS->SPIMRST = 0;

    return (0);
}


/*
 *  Erase Sector in Flash Memory
 *    Parameter:      adr:  Sector Address
 *    Return Value:   0 - OK,  1 - Failed
 */

int EraseSector (unsigned long adr)
{
    uint8_t u8Status;

    if ((adr < SPIM_DMM0_ADDR) || (adr >= (SPIM_DMM0_ADDR + SPIM_DMM_SIZE)))
        return (1);

    adr -= SPIM_DMM0_ADDR;

    while (SPIM0_S->CTL1 & SPIM_CTL1_SPIMEN_Msk);

    u8Status = SPIFLASH_WriteEnable();

    if ((u8Status & SR_WEL) == 0)
        return (1);

    // CS is active (Low)
    SPIM0_S->CTL1 = (SPIM0_S->CTL1 & (~SPIM_CTL1_SS_Msk)) | (0UL << SPIM_CTL1_SS_Pos);

    SPIM0_S->CTL0 = (SPIM0_S->CTL0 & ~(SPIM_CTL0_OPMODE_Msk | SPIM_CTL0_QDIODIR_Msk)) | ((SPIM_CTL0_OPMODE_IO << SPIM_CTL0_OPMODE_Pos) | (1UL << SPIM_CTL0_QDIODIR_Pos));

    // Send Sector Erase instruction
    SPIM_IO_Write(OPCODE_SE_4K);

    SPIM_IO_Write((adr >> 16) & 0xFF);
    SPIM_IO_Write((adr >>  8) & 0xFF);
    SPIM_IO_Write((adr      ) & 0xFF);

    // CS is inactive (High)
    SPIM0_S->CTL1 = (SPIM0_S->CTL1 & (~SPIM_CTL1_SS_Msk)) | (1UL << SPIM_CTL1_SS_Pos);

    u8Status = SPIFLASH_CheckStatus(SR_WIP, 0);

    return (0);
}


/*
 *  Erase Sectors in Flash Memory
 *    Parameter:      buf:  Sector Address List
 *                    sz:   List Size
 *                    fnc:  Function Code (0 - Sector Erase, Other - Reserved)
 *    Return Value:   0 - OK,  1 - Failed
 */

int EraseSectorEx (unsigned long *buf, unsigned long sz, unsigned long fnc)
{
    uint32_t u32Index = 0;

    if (fnc > 0)
        return (1);

    while (u32Index < sz)
    {
        if (EraseSector(buf[u32Index++]) != 0)
        {
            return (1);
        }
    }

    return (0);
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
    uint32_t u32Len;
    uint32_t u32WriteSize;
    __ALIGNED(8) uint8_t u8WriteBuffer[SPIFLASH_PAGE_SIZE];

    if ((adr < SPIM_DMM0_ADDR) || ((adr + sz) > (SPIM_DMM0_ADDR + SPIM_DMM_SIZE)))
        return (1);

    adr -= SPIM_DMM0_ADDR;

    while (sz > 0)
    {
        u32Len = sz;
        if (u32Len > SPIFLASH_PAGE_SIZE)
            u32Len = SPIFLASH_PAGE_SIZE;

        u32WriteSize = (u32Len + (8-1)) & ~(8-1);

        memset(u8WriteBuffer, 0xFF, u32WriteSize);
        memcpy(u8WriteBuffer, buf, u32Len);

        if (SPIFLASH_PageProgram(adr, u32WriteSize, u8WriteBuffer) != 0)
            return (1);

        adr += u32Len;
        buf += u32Len;
        sz  -= u32Len;
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
    uint32_t i, u32Len;
    uint32_t u32ReadSize;
    __ALIGNED(8) uint8_t u8ReadBuffer[SPIFLASH_PAGE_SIZE];

    if ((adr < SPIM_DMM0_ADDR) || ((adr + sz) > (SPIM_DMM0_ADDR + SPIM_DMM_SIZE)))
        return (adr);

    adr -= SPIM_DMM0_ADDR;

    while (sz > 0)
    {
        u32Len = sz;
        if (u32Len > SPIFLASH_PAGE_SIZE)
            u32Len = SPIFLASH_PAGE_SIZE;

        u32ReadSize = (u32Len + (8-1)) & ~(8-1);

        SPIFLASH_ReadData(adr, u32ReadSize, u8ReadBuffer);

        for (i = 0; i < u32Len; i++)
        {
            if (u8ReadBuffer[i] != buf[i])
            {
                return (adr + SPIM_DMM0_ADDR + i);      // Verification Failed (return address)
            }
        }

        adr += u32Len;
        buf += u32Len;
        sz  -= u32Len;
    }

    return (adr + SPIM_DMM0_ADDR);
}
