/* Public-edition staging: local UI additions and/or privacy review; see source manifest. */
#pragma once
#include "sdkconfig.h"
#include "lvgl.h"
#include "muse_state.h"
#if CONFIG_MUSE_BOARD_WAVESHARE_S3_175C
void muse_status_ui_build(lv_obj_t *face);
void muse_status_ui_update(muse_power_t power,bool wifi,uint32_t color,const char *detail);
void muse_status_ui_tick(bool visible,muse_mode_t mode);
#endif
