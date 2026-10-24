/* Public-edition staging: local UI additions and/or privacy review; see source manifest. */
/* Pixel microphone: an independent touch PTT, never a PMU key press. */
#pragma once
#include "lvgl.h"
#include "muse_pixel_controls.h"
#include <stdbool.h>

enum { MUSE_TOUCH_MIC_SIZE = 64, MUSE_TOUCH_MIC_Y = 184,
       MUSE_TOUCH_MIC_REPLY_BOTTOM = 142 };
typedef struct {
    lv_obj_t *obj;
    bool active, enabled, listening;
    void (*send)(bool);
} muse_touch_mic_t;

static inline void muse_touch_mic_release(muse_touch_mic_t *mic)
{
    if (!mic->active) return;
    mic->active = false;
    mic->send(false);
}
static void muse_touch_mic_event(lv_event_t *e)
{
    muse_touch_mic_t *mic = lv_event_get_user_data(e);
    switch (lv_event_get_code(e)) {
    case LV_EVENT_LONG_PRESSED:
        if (mic->enabled && !mic->active) {
            mic->active = true;
            mic->send(true);
            lv_obj_invalidate(mic->obj);
        }
        break;
    case LV_EVENT_RELEASED:
    case LV_EVENT_PRESS_LOST:
        muse_touch_mic_release(mic);
        lv_obj_invalidate(mic->obj);
        break;
    case LV_EVENT_DRAW_MAIN: {
        lv_event_stop_processing(e);
        lv_area_t box; lv_obj_get_coords(mic->obj, &box);
        muse_pixel_control_draw(lv_event_get_layer(e), &box, MUSE_ICON_MIC,
                                mic->active || mic->listening ? 0x64D4FF : 0xD6C0FF,
                                mic->active || mic->listening);
        break;
    }
    default: break;
    }
}
static inline void muse_touch_mic_build(muse_touch_mic_t *mic, lv_obj_t *parent, void (*send)(bool))
{
    *mic = (muse_touch_mic_t){.enabled=true,.send=send};
    mic->obj = lv_obj_create(parent);
    lv_obj_remove_style_all(mic->obj);
    lv_obj_set_size(mic->obj, MUSE_TOUCH_MIC_SIZE, MUSE_TOUCH_MIC_SIZE);
    lv_obj_align(mic->obj, LV_ALIGN_CENTER, 0, MUSE_TOUCH_MIC_Y);
    lv_obj_add_flag(mic->obj, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK);
    lv_obj_remove_flag(mic->obj, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_CHAIN |
                                    LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_obj_add_event_cb(mic->obj, muse_touch_mic_event, LV_EVENT_ALL, mic);
}
static inline void muse_touch_mic_update(muse_touch_mic_t *mic, bool visible, bool listening)
{
    mic->enabled = visible;
    if (!visible) muse_touch_mic_release(mic);
    lv_obj_set_flag(mic->obj, LV_OBJ_FLAG_HIDDEN, !visible);
    if (mic->listening != listening) {
        mic->listening = listening;
        lv_obj_invalidate(mic->obj);
    }
}
