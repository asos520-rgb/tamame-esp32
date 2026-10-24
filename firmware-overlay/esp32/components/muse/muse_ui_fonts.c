/* Public-edition staging: local UI additions and/or privacy review; see source manifest. */
#include "muse_ui_fonts.h"
#include "sdkconfig.h"
#include "muse_text.h"
#if CONFIG_MUSE_TC_FONT
LV_FONT_DECLARE(muse_font_noto_tc_16)
#endif
const lv_font_t *muse_ui_font(const lv_font_t *base)
{
#if CONFIG_MUSE_TC_FONT
    /* LVGL UI task owns this small cache. Original fonts and the caption font
     * remain const. Preserve ASCII/icon glyphs, use Noto only as fallback. */
    static const lv_font_t *keys[24];
    static lv_font_t fonts[24];
    static unsigned count;
    if(!base || base==&muse_font_noto_tc_16)return base;
    for(unsigned i=0;i<count;i++)if(keys[i]==base)return &fonts[i];
    if(count==24)return &muse_font_noto_tc_16;
    unsigned i=count++;keys[i]=base;fonts[i]=*base;
    fonts[i].fallback=&muse_font_noto_tc_16;
    if(fonts[i].line_height<muse_font_noto_tc_16.line_height) {
        fonts[i].line_height=muse_font_noto_tc_16.line_height;
        fonts[i].base_line=muse_font_noto_tc_16.base_line;
    }
    return &fonts[i];
#else
    return base;
#endif
}

const char *muse_ui_showable(const char *text,char *buffer,size_t capacity)
{
#if CONFIG_MUSE_TC_FONT
    (void)buffer;(void)capacity;return text; /* UI font fallback handles CJK */
#else
    return muse_text_showable(text,buffer,capacity);
#endif
}
