# Board: Nuvoton [NuMaker-X-M55M1D](https://www.nuvoton.com/products/gui-solution/gui-platform/numaker-x-m55m1d/index.html)

## Default Board Layer

Device: **M55M1H2LJAE**

System Core Clock: **220 MHz**


### System Configuration

| System resource       | Setting
|:----------------------|:-------------------------------------------------
| Heap                  | 64 kB (configured in the M55M1.scf linker script)
| Stack (MSP)           |  4 kB (configured in the M55M1.scf linker script)

### STDIO mapping

**STDIO** is routed to Virtual COM port on the Nu-Link2-Me debugger (using **UART0** peripheral)

### CMSIS-Driver mapping

| CMSIS-Driver          | Peripheral | Board connector/component    | Connection
|:----------------------|:-----------|:-----------------------------|:-----------------------
| Driver_USBD1          | HSUSBD     | HSUSB (J13)                  | CMSIS_USB_Device
| CMSIS-Driver VIO      | GPIO       | LEDs and buttons             | CMSIS_VIO
| Driver_vStreamVideoIn | CCAP       | CMOS-720P V1.0               | CMSIS_VSTREAM_VIDEO_IN
| Driver_vStreamVideoOut| EBI        | NuMaker-TFT-LCD5 V1.2        | CMSIS_VSTREAM_VIDEO_OUT

### CMSIS-Driver Virtual I/O mapping

| CMSIS-Driver VIO      | Board component
|:----------------------|:---------------
| vioBUTTON0            | BTN0
| vioBUTTON1            | BTN1
| vioLED0               | LED red
| vioLED1               | LED green
| vioLED2               | LED yellow

### CMSIS-Driver vStream configuration

| Driver                 | Stream Format Description
|:-----------------------|:----------------------------------
| Driver_vStreamVideoIn  | RGB565 frame, resolution 320 x 240
| Driver_vStreamVideoOut | RGB565 frame, resolution 800 x 480
