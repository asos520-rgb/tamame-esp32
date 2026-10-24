/* Public-edition staging: local UI additions and/or privacy review; see source manifest. */
/* Font fallback belongs to UI chrome; reply text/font selection stays unchanged. */
#pragma once
#include "lvgl.h"
#include <stddef.h>
const lv_font_t *muse_ui_font(const lv_font_t *base);

const char *muse_ui_showable(const char *text,char *buffer,size_t capacity);
