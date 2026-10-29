/***********************************************************************/
/*  This file is part of the uVision/ARM development tools             */
/*  Copyright (c) 2006-2023 Keil - An ARM Company.                     */
/*  All rights reserved.                                               */
/***********************************************************************/
/*                                                                     */
/*  FlashDev.C:  Device Description for M55M1 External SPI Flash       */
/*                                                                     */
/***********************************************************************/

#include "..\..\FlashOS.h"      // FlashOS Structures


#ifdef M55M1_SERIES

#define DEVICENAME_SPI_32M      "M55M1 SPI Flash"

#endif

#ifdef M5531_SERIES

#define DEVICENAME_SPI_32M      "M5531 SPI Flash"

#endif


#ifdef FLASH_SPIM_32M
struct FlashDevice const FlashDevice  =  {
    FLASH_DRV_VERS,             // Driver Version, do not modify!
    DEVICENAME_SPI_32M,         // Device Name
    EXTSPI,                     // Device Type
    0x82000000,                 // Device Start Address
    0x02000000,                 // Device Size in Bytes (32MB)
    256,                        // Programming Page Size
    0,                          // Reserved, must be 0
    0xFF,                       // Initial Content of Erased Memory
    1000,                       // Program Page Timeout 1000 mSec
    1000,                       // Erase Sector Timeout 1000 mSec

// Specify Size and Address of Sectors
    0x1000, 0x000000,           // Sector Size 4kB (8192 Sectors)
    SECTOR_END
};
#endif
