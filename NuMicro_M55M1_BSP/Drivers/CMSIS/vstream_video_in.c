/* HM1055 video input for NuMaker-X-M55M1D.
 * Each block is one 320 x 240 packed RGB565 frame (153600 bytes).
 * The application supplies at least two cache-line-aligned frame blocks.
 */
#include <stddef.h>
#include <stdint.h>

#include "cmsis_vstream.h"
#include "NuMicro.h"
#include "ImageSensor.h"

#define VIDEO_WIDTH       320U
#define VIDEO_HEIGHT      240U
#define VIDEO_FRAME_BYTES (VIDEO_WIDTH * VIDEO_HEIGHT * 2U)

static uint8_t *frame_buffer;
static uint32_t frame_count;
static uint32_t frame_index;
static vStreamEvent_t event_callback;
static volatile uint8_t initialized;
static volatile uint8_t active;
static volatile uint8_t pending;
static volatile uint8_t ready;
static volatile uint8_t leased;
static volatile uint8_t single;
static volatile uint8_t eos;
static volatile uint8_t overflow;

static void frame_complete(void)
{
    uint32_t events;

    if ((active == 0U) || (pending == 0U)) {
        return;
    }
    pending = 0U;
#ifndef VIDEO_IN_MEMORY_NON_CACHEABLE
    SCB_InvalidateDCache_by_Addr(frame_buffer + (frame_index * VIDEO_FRAME_BYTES),
                                 (int32_t)VIDEO_FRAME_BYTES);
#endif
    ready = 1U;
    events = VSTREAM_EVENT_DATA;
    if (single != 0U) {
        active = 0U;
        eos = 1U;
        events |= VSTREAM_EVENT_EOS;
    }
    if (event_callback != NULL) {
        event_callback(events);
    }
}

static int32_t video_initialize(vStreamEvent_t callback)
{
    if (initialized != 0U) {
        event_callback = callback;
        return VSTREAM_OK;
    }
    if (ImageSensor_Init() != 0) {
        return VSTREAM_ERROR;
    }
    if (ImageSensor_Config(eIMAGE_FMT_RGB565, VIDEO_WIDTH, VIDEO_HEIGHT, true) != 0) {
        return VSTREAM_ERROR;
    }
    event_callback = callback;
    frame_buffer = NULL;
    frame_count = 0U;
    initialized = 1U;
    ImageSensor_SetFrameCallback(frame_complete);
    return VSTREAM_OK;
}

static int32_t video_stop(void)
{
    if (initialized == 0U) {
        return VSTREAM_ERROR;
    }
    NVIC_DisableIRQ(CCAP_IRQn);
    active = 0U;
    pending = 0U;
    ready = 0U;
    leased = 0U;
    eos = 0U;
    (void)CCAP_Stop(CCAP_DISABLE);
    CCAP->INTSTS = CCAP_INTSTS_VINTF_Msk;
    NVIC_ClearPendingIRQ(CCAP_IRQn);
    NVIC_EnableIRQ(CCAP_IRQn);
    return VSTREAM_OK;
}

static int32_t video_uninitialize(void)
{
    if (initialized == 0U) {
        return VSTREAM_OK;
    }
    (void)video_stop();
    ImageSensor_SetFrameCallback(NULL);
    NVIC_DisableIRQ(CCAP_IRQn);
    frame_buffer = NULL;
    frame_count = 0U;
    event_callback = NULL;
    initialized = 0U;
    return VSTREAM_OK;
}

static int32_t video_set_buf(void *buf, uint32_t buf_size, uint32_t block_size)
{
    if ((initialized == 0U) || (active != 0U) || (ready != 0U) || (buf == NULL) ||
        (((uintptr_t)buf & (DCACHE_LINE_SIZE - 1U)) != 0U) ||
        (block_size != VIDEO_FRAME_BYTES)) {
        return VSTREAM_ERROR_PARAMETER;
    }
    frame_buffer = (uint8_t *)buf;
    frame_count = buf_size / block_size;
    frame_index = 0U;
    ready = 0U;
    leased = 0U;
    return VSTREAM_OK;
}

static int32_t video_start(uint32_t mode)
{
    if ((initialized == 0U) || (frame_buffer == NULL) || (active != 0U) || (ready != 0U) ||
        ((mode != VSTREAM_MODE_CONTINUOUS) && (mode != VSTREAM_MODE_SINGLE))) {
        return VSTREAM_ERROR_PARAMETER;
    }
    frame_index = 0U;
    ready = 0U;
    leased = 0U;
    eos = 0U;
    overflow = 0U;
    single = (mode == VSTREAM_MODE_SINGLE) ? 1U : 0U;
#ifndef VIDEO_IN_MEMORY_NON_CACHEABLE
    SCB_CleanInvalidateDCache_by_Addr(frame_buffer, (int32_t)VIDEO_FRAME_BYTES);
#endif
    active = 1U;
    pending = 1U;
    if (ImageSensor_TriggerCapture((uint32_t)(uintptr_t)frame_buffer) != 0) {
        pending = 0U;
        active = 0U;
        return VSTREAM_ERROR;
    }
    return VSTREAM_OK;
}

static void *video_get_block(void)
{
    if (ready == 0U) {
        return NULL;
    }
    leased = 1U;
    return frame_buffer + (frame_index * VIDEO_FRAME_BYTES);
}

static int32_t video_release_block(void)
{
    if ((ready == 0U) || (leased == 0U)) {
        return VSTREAM_ERROR;
    }
    ready = 0U;
    leased = 0U;
    if (active != 0U) {
        frame_index = (frame_index + 1U) % frame_count;
#ifndef VIDEO_IN_MEMORY_NON_CACHEABLE
        SCB_CleanInvalidateDCache_by_Addr(frame_buffer + (frame_index * VIDEO_FRAME_BYTES),
                                          (int32_t)VIDEO_FRAME_BYTES);
#endif
        pending = 1U;
        if (ImageSensor_TriggerCapture((uint32_t)(uintptr_t)
                                       (frame_buffer + (frame_index * VIDEO_FRAME_BYTES))) != 0) {
            pending = 0U;
            active = 0U;
            return VSTREAM_ERROR;
        }
    }
    return VSTREAM_OK;
}

static vStreamStatus_t video_get_status(void)
{
    vStreamStatus_t status = {0};
    status.active = active;
    status.overflow = overflow;
    status.eos = eos;
    overflow = 0U;
    eos = 0U;
    return status;
}

vStreamDriver_t Driver_vStreamVideoIn = {
    video_initialize,
    video_uninitialize,
    video_set_buf,
    video_start,
    video_stop,
    video_get_block,
    video_release_block,
    video_get_status
};
