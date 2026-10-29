/* LT7381 video output for NuMaker-X-M55M1D.
 * Each block is one 800 x 480 RGB565 frame (768000 bytes).
 * The application supplies one cache-line-aligned frame block
 * in memory accessible to PDMA.
 */
#include <stddef.h>
#include <stdint.h>

#include "cmsis_vstream.h"
#include "NuMicro.h"
#include "Display.h"

#define VIDEO_WIDTH       800U
#define VIDEO_HEIGHT      480U
#define VIDEO_FRAME_BYTES (VIDEO_WIDTH * VIDEO_HEIGHT * sizeof(uint16_t))

static uint8_t *frame_buffer;
static uint32_t frame_count;
static uint32_t frame_index;
static vStreamEvent_t event_callback;
static uint8_t initialized;
static uint8_t active;
static uint8_t leased;
static uint8_t pending;
static uint8_t single;
static uint8_t eos;

static int32_t video_initialize(vStreamEvent_t callback)
{
    if (initialized == 0U) {
        if (Display_Init() != 0) {
            return VSTREAM_ERROR;
        }
        initialized = 1U;
    }
    event_callback = callback;
    return VSTREAM_OK;
}

static int32_t video_stop(void)
{
    if (initialized == 0U) {
        return VSTREAM_ERROR;
    }
    active = 0U;
    leased = 0U;
    pending = 0U;
    eos = 0U;
    return VSTREAM_OK;
}

static int32_t video_uninitialize(void)
{
    if (initialized != 0U) {
        (void)video_stop();
    }
    frame_buffer = NULL;
    frame_count = 0U;
    event_callback = NULL;
    initialized = 0U;
    return VSTREAM_OK;
}

static int32_t video_set_buf(void *buf, uint32_t buf_size, uint32_t block_size)
{
    if ((initialized == 0U) || (active != 0U) || (leased != 0U) ||
        (pending != 0U) || (buf == NULL) ||
        (((uintptr_t)buf & (DCACHE_LINE_SIZE - 1U)) != 0U) ||
        (block_size != VIDEO_FRAME_BYTES) || (buf_size < block_size)) {
        return VSTREAM_ERROR_PARAMETER;
    }
    frame_buffer = (uint8_t *)buf;
    frame_count = buf_size / block_size;
    frame_index = 0U;
    leased = 0U;
    return VSTREAM_OK;
}

static void video_send_block(void)
{
    static const S_DISP_RECT full_screen = {0U, 0U, VIDEO_WIDTH - 1U, VIDEO_HEIGHT - 1U};
    uint32_t events = VSTREAM_EVENT_DATA;

#ifndef VIDEO_OUT_MEMORY_NON_CACHEABLE
    SCB_CleanDCache_by_Addr(frame_buffer + (frame_index * VIDEO_FRAME_BYTES),
                            (int32_t)VIDEO_FRAME_BYTES);
#endif
    Display_FillRect((uint16_t *)(frame_buffer + (frame_index * VIDEO_FRAME_BYTES)),
                     &full_screen, 1);
    pending = 0U;
    leased = 0U;
    frame_index = (frame_index + 1U) % frame_count;
    if (single != 0U) {
        active = 0U;
        eos = 1U;
        events |= VSTREAM_EVENT_EOS;
    }
    if (event_callback != NULL) {
        event_callback(events);
    }
}

static int32_t video_start(uint32_t mode)
{
    if ((initialized == 0U) || (frame_buffer == NULL) || (active != 0U) ||
        ((mode != VSTREAM_MODE_CONTINUOUS) && (mode != VSTREAM_MODE_SINGLE))) {
        return VSTREAM_ERROR_PARAMETER;
    }
    if ((Disaplay_GetLCDWidth() != VIDEO_WIDTH) ||
        (Disaplay_GetLCDHeight() != VIDEO_HEIGHT)) {
        return VSTREAM_ERROR;
    }
    eos = 0U;
    single = (mode == VSTREAM_MODE_SINGLE) ? 1U : 0U;
    active = 1U;
    if (pending != 0U) {
        video_send_block();
    }
    return VSTREAM_OK;
}

static void *video_get_block(void)
{
    if ((initialized == 0U) || (frame_buffer == NULL) ||
        (leased != 0U) || (pending != 0U)) {
        return NULL;
    }
    leased = 1U;
    return frame_buffer + (frame_index * VIDEO_FRAME_BYTES);
}

static int32_t video_release_block(void)
{
    if (leased == 0U) {
        return VSTREAM_ERROR;
    }
    if (active == 0U) {
        leased = 0U;
        pending = 1U;
    } else {
        video_send_block();
    }
    return VSTREAM_OK;
}

static vStreamStatus_t video_get_status(void)
{
    vStreamStatus_t status = {0};
    status.active = active;
    status.eos = eos;
    eos = 0U;
    return status;
}

vStreamDriver_t Driver_vStreamVideoOut = {
    video_initialize,
    video_uninitialize,
    video_set_buf,
    video_start,
    video_stop,
    video_get_block,
    video_release_block,
    video_get_status
};
