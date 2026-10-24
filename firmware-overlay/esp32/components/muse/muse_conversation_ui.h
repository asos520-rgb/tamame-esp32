/* Public-edition staging: local UI additions and/or privacy review; see source manifest. */
/* SPDX-License-Identifier: Apache-2.0 */
#pragma once
#include "sdkconfig.h"
#include "lvgl.h"
#include "muse_state.h"
#if CONFIG_MUSE_BOARD_WAVESHARE_S3_175C
void muse_conversation_ui_build(lv_obj_t *face,const lv_font_t *font);
void muse_conversation_ui_tick(bool visible,muse_mode_t mode);
#else
#define muse_conversation_ui_build(face,font) ((void)0)
#define muse_conversation_ui_tick(visible,mode) ((void)0)
#endif
