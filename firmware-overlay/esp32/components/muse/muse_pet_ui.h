/* Public-edition staging: local UI additions and/or privacy review; see source manifest. */
/* SPDX-License-Identifier: Apache-2.0 */
#pragma once
#include "sdkconfig.h"
#include "muse_pixel.h"
#include "lvgl.h"
#if CONFIG_MUSE_BOARD_WAVESHARE_S3_175C
void muse_pet_ui_build(lv_obj_t *face,lv_obj_t *canvas,const lv_font_t *font);
void muse_pet_ui_tick(bool visible,muse_mode_t mode,float now);
void muse_pet_ui_pose(muse_pose_t *pose,float now);
void muse_pet_ui_place(int base_y);
bool muse_pet_ui_hint_active(void);
#else
#define muse_pet_ui_build(face,canvas,font) ((void)0)
#define muse_pet_ui_tick(visible,mode,now) ((void)0)
#define muse_pet_ui_pose(pose,now) ((void)0)
#define muse_pet_ui_place(y) ((void)0)
#endif
