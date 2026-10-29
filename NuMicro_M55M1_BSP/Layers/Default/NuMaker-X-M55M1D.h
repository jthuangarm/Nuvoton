/*
 * Copyright 2026 Arm Limited and/or its affiliates.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef NUMAKER_X_M55M1D_H_
#define NUMAKER_X_M55M1D_H_

#include "Driver_USBD.h"
#include "cmsis_vstream.h"

// CMSIS Driver instances of Board peripherals
#define CMSIS_DRIVER_USBD       1       // Driver_USBD1
#define CMSIS_DRIVER_USBD_HS    1       // CMSIS Driver USB Device high-speed support

// CMSIS Driver instance for STDIO retarget
#define RETARGET_STDIO_UART     0

// CMSIS Drivers
extern ARM_DRIVER_USBD          ARM_Driver_USBD_(CMSIS_DRIVER_USBD);    // USB Device
extern vStreamDriver_t          Driver_vStreamVideoIn;                  // Video In Stream (Camera: 320 x 240 RGB565)
extern vStreamDriver_t          Driver_vStreamVideoOut;                 // Video Out Stream (LCD: 800 x 480 RGB565)

#ifdef   CMSIS_shield_header
#include CMSIS_shield_header
#endif

#endif // NUMAKER_X_M55M1D_H_
