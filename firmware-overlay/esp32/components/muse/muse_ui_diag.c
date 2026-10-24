/* Public-edition staging: local UI additions and/or privacy review; see source manifest. */
/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* Temporary freeze diagnostics. No UI/display locks, object access, NVS,
 * credentials, reply text, reset, recovery or control behavior in the observer. */
#include "muse_ui_diag.h"
#if CONFIG_MUSE_BOARD_WAVESHARE_S3_175C
#include "muse_ui_diag_watch.h"
#include "muse_mem.h"
#include "esp_attr.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/idf_additions.h"
static const char *TAG = "ui_progress";
/* Each 32-bit counter has exactly one writer. Samples are diagnostic only;
 * they can straddle an event and must not be used for control decisions. */
static volatile uint32_t s_frame, s_render_start, s_render_done;
static volatile uint32_t s_flush_start, s_flush_done, s_wait_start, s_wait_done;
static volatile uint32_t s_transfer_done;
static bool s_started;
void muse_ui_diag_frame(void) { ++s_frame; }
void IRAM_ATTR muse_ui_diag_transfer_done_from_isr(void) { ++s_transfer_done; }
static void event(lv_event_t *e)
{
    switch (lv_event_get_code(e)) {
    case LV_EVENT_RENDER_START: ++s_render_start; break;
    case LV_EVENT_RENDER_READY: ++s_render_done; break;
    case LV_EVENT_FLUSH_START: ++s_flush_start; break;
    case LV_EVENT_FLUSH_FINISH: ++s_flush_done; break;
    case LV_EVENT_FLUSH_WAIT_START: ++s_wait_start; break;
    case LV_EVENT_FLUSH_WAIT_FINISH: ++s_wait_done; break;
    default: break;
    }
}
static void observe(void *arg)
{
    (void)arg;
    muse_ui_diag_watch_t watch = {0};
    bool was_stalled = false;
    unsigned sample = 0;
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(2000));
        uint32_t frame = s_frame;
        bool stalled = muse_ui_diag_stalled(&watch, frame, (uint32_t)(esp_timer_get_time() / 1000));
        /* Normal once per 30 seconds; stale every 2 seconds, even with LVGL
         * blocked. Counters distinguish render/flush/transport waits. */
        if (stalled || was_stalled || ++sample % 15 == 0) {
            ESP_LOGI(TAG, "%s frame=%lu render=%lu/%lu flush=%lu/%lu wait=%lu/%lu tx_done=%lu",
                stalled ? "STALLED" : was_stalled ? "RECOVERED" : "OK",
                (unsigned long)frame, (unsigned long)s_render_start, (unsigned long)s_render_done,
                (unsigned long)s_flush_start, (unsigned long)s_flush_done,
                (unsigned long)s_wait_start, (unsigned long)s_wait_done, (unsigned long)s_transfer_done);
        }
        was_stalled = stalled;
    }
}
void muse_ui_diag_start(lv_display_t *display)
{
    if (s_started) return;
    s_started = true;
    lv_display_add_event_cb(display, event, LV_EVENT_ALL, NULL);
    /* PSRAM stack preserves scarce internal DMA memory. No settings/flash
     * calls from this task; creation failure leaves ordinary UI untouched. */
    if (xTaskCreateWithCaps(observe, "ui_progress", 3072, NULL, 1, NULL, MUSE_BIG_CAPS) != pdPASS) {
        ESP_LOGW(TAG, "observer unavailable: task allocation failed");
    } else {
        ESP_LOGI(TAG, "observer enabled; diagnostic image, no automatic recovery");
    }
}
#endif
