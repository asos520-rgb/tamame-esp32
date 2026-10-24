/* Public-edition staging: local UI additions and/or privacy review; see source manifest. */
#pragma once
#include "lvgl.h"
typedef struct {lv_obj_t *frame,*label;int height;} muse_hint_panel_t;
void muse_hint_panel_create(muse_hint_panel_t *panel,lv_obj_t *parent,const lv_font_t *font);
void muse_hint_panel_sync(muse_hint_panel_t *panel,bool visible);
