/***********************************************************************/
/*  This file is part of the uVision/ARM development tools             */
/*  Copyright (c) 2006-2023 Keil - An ARM Company.                     */
/*  All rights reserved.                                               */
/***********************************************************************/
/*                                                                     */
/*  FlashDev.C:  Device Description for M55M1 Flash                    */
/*                                                                     */
/***********************************************************************/

#include "..\FlashOS.h"         // FlashOS Structures


//#ifdef M55M1_SERIES

#define DEVICENAME_AP_2M        "M55M1 APROM 2MB"
#define DEVICENAME_AP1_1M       "M55M1 APROM BANK1 1MB"

#define DEVICENAME_LD_8         "M55M1 LDROM 8kB"

//#endif


#ifdef FLASH_APROM_2M
struct FlashDevice const FlashDevice  =  {
    FLASH_DRV_VERS,             // Driver Version, do not modify!
    DEVICENAME_AP_2M,           // Device Name
    ONCHIP,                     // Device Type
    0x00100000,                 // Device Start Address
    0x00200000,                 // Device Size in Bytes (2MB)
    512,                        // Programming Page Size
    0,                          // Reserved, must be 0
    0xFF,                       // Initial Content of Erased Memory
    100,                        // Program Page Timeout 100 mSec
    200,                        // Erase Sector Timeout 200 mSec

// Specify Size and Address of Sectors
    0x2000, 0x000000,           // Sector Size 8kB (256 Sectors)
    SECTOR_END
};
#endif


#ifdef FLASH_APROM_BANK1_1M
struct FlashDevice const FlashDevice  =  {
    FLASH_DRV_VERS,             // Driver Version, do not modify!
    DEVICENAME_AP1_1M,          // Device Name
    ONCHIP,                     // Device Type
    0x00100000,                 // Device Start Address
    0x00100000,                 // Device Size in Bytes (1MB)
    512,                        // Programming Page Size
    0,                          // Reserved, must be 0
    0xFF,                       // Initial Content of Erased Memory
    100,                        // Program Page Timeout 100 mSec
    200,                        // Erase Sector Timeout 200 mSec

// Specify Size and Address of Sectors
    0x2000, 0x000000,           // Sector Size 8kB (128 Sectors)
    SECTOR_END
};
#endif


#ifdef FLASH_LDROM_8K
struct FlashDevice const FlashDevice  =  {
    FLASH_DRV_VERS,             // Driver Version, do not modify!
    DEVICENAME_LD_8,            // Device Name
    ONCHIP,                     // Device Type
    0x0F100000,                 // Device Start Address
    0x00002000,                 // Device Size in Bytes (8kB)
    512,                        // Programming Page Size
    0,                          // Reserved, must be 0
    0xFF,                       // Initial Content of Erased Memory
    100,                        // Program Page Timeout 100 mSec
    200,                        // Erase Sector Timeout 200 mSec

// Specify Size and Address of Sectors
    0x2000, 0x000000,           // Sector Size 8kB (1 Sectors)
    SECTOR_END
};
#endif                          // end FLASH_LDROM
