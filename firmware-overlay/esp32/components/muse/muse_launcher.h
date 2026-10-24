/* Public-edition staging: local UI additions and/or privacy review; see source manifest. */
#pragma once
#include "sdkconfig.h"
#include "lvgl.h"
#include "muse_state.h"
#if CONFIG_MUSE_BOARD_WAVESHARE_S3_175C
void muse_launcher_build(lv_obj_t *face);
void muse_launcher_add(unsigned id,lv_obj_t *button);
void muse_launcher_modal(lv_obj_t *modal);
void muse_launcher_balance(lv_obj_t *label);
void muse_launcher_before(bool visible,muse_mode_t mode,float now);
void muse_launcher_after(void);
bool muse_launcher_entries_visible(void);
void muse_launcher_close(void);
#endif
