/* Public-edition staging: local UI additions and/or privacy review; see source manifest. */
/* SPDX-License-Identifier: Apache-2.0 */
#pragma once
#include "sdkconfig.h"
#include "muse_state.h"
#include "lvgl.h"
#if CONFIG_MUSE_BOARD_WAVESHARE_S3_175C
void muse_style_ui_build(lv_obj_t *face,lv_obj_t *canvas);
void muse_style_ui_tick(bool visible,muse_mode_t mode);
/* Local serial diagnostic UI actions; consumed on LVGL task. */
void muse_style_ui_request(unsigned action,unsigned value);
#else
#define muse_style_ui_build(face,canvas) ((void)0)
#define muse_style_ui_tick(visible,mode) ((void)0)
#endif
