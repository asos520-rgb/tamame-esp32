#include "muse_i18n.h"
/* Public-edition staging: local UI additions and/or privacy review; see source manifest. */
/* SPDX-License-Identifier: Apache-2.0 */
#include "muse_conversation_ui.h"
#if CONFIG_MUSE_BOARD_WAVESHARE_S3_175C
#include "muse_recent.h"
#include "muse_launcher.h"
#include "muse_pet_art.h"
#include "muse_ui_fonts.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
static lv_obj_t *s_entry,*s_modal,*s_panel,*s_title,*s_note,*s_list,*s_scroller,*s_text,*s_back;
static const lv_font_t *s_font;
static bool s_allowed,s_open;
static lv_obj_t *text_label(lv_obj_t *parent,const char *text) {
    lv_obj_t *o=lv_label_create(parent);lv_obj_set_style_text_font(o,s_font,0);
    lv_obj_set_style_text_color(o,lv_color_hex(0xF4F0E7),0);lv_label_set_text(o,text);
    lv_obj_remove_flag(o,LV_OBJ_FLAG_CLICKABLE);return o;
}
static void close_recent(lv_event_t *e){(void)e;s_open=false;lv_obj_add_flag(s_modal,LV_OBJ_FLAG_HIDDEN);if(s_text)lv_label_set_text(s_text,"");}
static void list_show(lv_event_t *e);
static void detail_show(lv_event_t *e) {
    uint32_t serial=(uint32_t)(uintptr_t)lv_event_get_user_data(e);muse_recent_meta_t meta;
    char *copy=muse_recent_copy(serial,&meta);
    if(!copy){lv_label_set_text(s_note,muse_i18n_text(MUSE_KEY_s193));return;}
    lv_label_set_text(s_title,muse_i18n_text(MUSE_KEY_s188));lv_label_set_text(s_note,muse_i18n_text(MUSE_KEY_s187));
    lv_label_set_text(s_text,copy[0]?copy:muse_i18n_text(MUSE_KEY_s192));free(copy);
    lv_obj_add_flag(s_list,LV_OBJ_FLAG_HIDDEN);lv_obj_remove_flag(s_scroller,LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(s_back,LV_OBJ_FLAG_HIDDEN);lv_obj_scroll_to_y(s_scroller,0,LV_ANIM_OFF);
}
static void list_show(lv_event_t *e) {
    (void)e;if(!s_allowed)return;
    muse_state_poke();s_open=true;lv_label_set_text(s_title,muse_i18n_text(MUSE_KEY_s190));
    lv_label_set_text(s_note,muse_recent_failed()?muse_i18n_text(MUSE_KEY_s191):muse_i18n_text(MUSE_KEY_s189));
    lv_label_set_text(s_text,"");lv_obj_clean(s_list);lv_obj_remove_flag(s_list,LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(s_scroller,LV_OBJ_FLAG_HIDDEN);lv_obj_add_flag(s_back,LV_OBJ_FLAG_HIDDEN);
    unsigned count=muse_recent_count();
    if(!count){lv_obj_t *empty=text_label(s_list,muse_i18n_text(MUSE_KEY_s194));lv_obj_set_width(empty,274);lv_obj_set_style_text_align(empty,LV_TEXT_ALIGN_CENTER,0);}
    for(unsigned i=0;i<count;i++) {
        muse_recent_meta_t meta;char preview[128];if(!muse_recent_summary(i,&meta,preview,sizeof preview))continue;
        for(char *p=preview;*p;p++)if(*p=='\n'||*p=='\r')*p=' ';
        lv_obj_t *row=lv_button_create(s_list);lv_obj_set_size(row,286,80);
        lv_obj_set_style_bg_color(row,lv_color_hex(0x253D4A),0);lv_obj_set_style_radius(row,12,0);
        lv_obj_remove_flag(row,LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_t *badge=text_label(row,muse_i18n_text(MUSE_KEY_s187));lv_obj_set_pos(badge,0,-4);lv_obj_set_style_text_color(badge,lv_color_hex(0x8AC1CE),0);
        lv_obj_t *line=text_label(row,preview[0]?preview:muse_i18n_text(MUSE_KEY_s192));lv_obj_set_size(line,258,40);lv_obj_set_pos(line,0,22);lv_label_set_long_mode(line,LV_LABEL_LONG_MODE_DOTS);
        lv_obj_add_event_cb(row,detail_show,LV_EVENT_CLICKED,(void*)(uintptr_t)meta.serial);
    }
    lv_obj_scroll_to_y(s_list,0,LV_ANIM_OFF);lv_obj_remove_flag(s_modal,LV_OBJ_FLAG_HIDDEN);lv_obj_move_foreground(s_modal);
}
static void draw_entry(lv_event_t *e) {
    lv_event_stop_processing(e);lv_obj_t *o=lv_event_get_target_obj(e);lv_area_t box;lv_obj_get_coords(o,&box);
    lv_draw_rect_dsc_t d;lv_draw_rect_dsc_init(&d);d.radius=0;d.border_width=0;d.bg_opa=LV_OPA_COVER;
    for(int y=0;y<16;y++)for(int x=0;x<16;x++) {
        uint32_t rgb=muse_pet_art_pixel(9,x,y);if(!rgb)continue;d.bg_color=lv_color_hex(rgb);
        lv_area_t a={box.x1+x*2,box.y1+y*2,box.x1+x*2+1,box.y1+y*2+1};lv_draw_rect(lv_event_get_layer(e),&d,&a);
    }
}
void muse_conversation_ui_build(lv_obj_t *face,const lv_font_t *font) {
    (void)font;s_font=muse_ui_font(&lv_font_montserrat_16);
    s_entry=lv_button_create(face);lv_obj_set_size(s_entry,46,46);lv_obj_align(s_entry,LV_ALIGN_CENTER,-128,132);
    lv_obj_set_style_bg_color(s_entry,lv_color_hex(0x222734),0);lv_obj_set_style_radius(s_entry,LV_RADIUS_CIRCLE,0);
    lv_obj_set_style_border_width(s_entry,1,0);lv_obj_set_style_border_color(s_entry,lv_color_hex(0x596176),0);
    lv_obj_t *icon=lv_obj_create(s_entry);lv_obj_remove_style_all(icon);lv_obj_set_size(icon,32,32);lv_obj_center(icon);
    lv_obj_remove_flag(icon,LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);lv_obj_add_event_cb(icon,draw_entry,LV_EVENT_DRAW_MAIN|LV_EVENT_PREPROCESS,NULL);
    lv_obj_add_event_cb(s_entry,list_show,LV_EVENT_CLICKED,NULL);
    muse_launcher_add(4,s_entry);
    s_modal=lv_obj_create(lv_layer_top());lv_obj_remove_style_all(s_modal);lv_obj_set_size(s_modal,lv_pct(100),lv_pct(100));
    lv_obj_set_style_bg_color(s_modal,lv_color_black(),0);lv_obj_set_style_bg_opa(s_modal,LV_OPA_80,0);
    lv_obj_add_flag(s_modal,LV_OBJ_FLAG_HIDDEN|LV_OBJ_FLAG_CLICKABLE);lv_obj_remove_flag(s_modal,LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(s_modal,close_recent,LV_EVENT_CLICKED,NULL);
    muse_launcher_modal(s_modal);
    s_panel=lv_obj_create(s_modal);lv_obj_remove_style_all(s_panel);lv_obj_set_size(s_panel,334,354);lv_obj_center(s_panel);
    lv_obj_set_style_bg_color(s_panel,lv_color_hex(0x192731),0);lv_obj_set_style_bg_opa(s_panel,LV_OPA_COVER,0);
    lv_obj_set_style_border_color(s_panel,lv_color_hex(0x7DADCB),0);lv_obj_set_style_border_width(s_panel,2,0);lv_obj_set_style_radius(s_panel,20,0);
    lv_obj_add_flag(s_panel,LV_OBJ_FLAG_CLICKABLE);lv_obj_remove_flag(s_panel,LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_EVENT_BUBBLE);
    s_title=text_label(s_panel,muse_i18n_text(MUSE_KEY_s190));lv_obj_align(s_title,LV_ALIGN_TOP_MID,0,16);
    lv_obj_t *close=lv_button_create(s_panel);lv_obj_set_size(close,36,32);lv_obj_align(close,LV_ALIGN_TOP_RIGHT,-8,8);lv_obj_add_event_cb(close,close_recent,LV_EVENT_CLICKED,NULL);
    lv_obj_t *mark=lv_label_create(close);lv_label_set_text(mark,LV_SYMBOL_CLOSE);lv_obj_center(mark);
    s_note=text_label(s_panel,"");lv_obj_set_size(s_note,286,42);lv_obj_set_pos(s_note,24,52);lv_obj_set_style_text_color(s_note,lv_color_hex(0xAFC2CF),0);lv_obj_set_style_text_align(s_note,LV_TEXT_ALIGN_CENTER,0);
    s_list=lv_obj_create(s_panel);lv_obj_remove_style_all(s_list);lv_obj_set_size(s_list,300,236);lv_obj_set_pos(s_list,17,102);
    lv_obj_set_flex_flow(s_list,LV_FLEX_FLOW_COLUMN);lv_obj_set_style_pad_row(s_list,8,0);lv_obj_set_style_pad_all(s_list,5,0);lv_obj_set_scroll_dir(s_list,LV_DIR_VER);
    s_scroller=lv_obj_create(s_panel);lv_obj_remove_style_all(s_scroller);lv_obj_set_size(s_scroller,290,212);lv_obj_set_pos(s_scroller,22,112);
    lv_obj_set_scroll_dir(s_scroller,LV_DIR_VER);lv_obj_set_style_pad_all(s_scroller,6,0);lv_obj_add_flag(s_scroller,LV_OBJ_FLAG_HIDDEN);
    s_text=text_label(s_scroller,"");lv_obj_set_width(s_text,270);lv_label_set_long_mode(s_text,LV_LABEL_LONG_MODE_WRAP);
    s_back=lv_button_create(s_panel);lv_obj_set_size(s_back,58,30);lv_obj_set_pos(s_back,12,8);lv_obj_add_event_cb(s_back,list_show,LV_EVENT_CLICKED,NULL);
    lv_obj_t *back=text_label(s_back,muse_i18n_text(MUSE_KEY_s086));lv_obj_center(back);lv_obj_add_flag(s_back,LV_OBJ_FLAG_HIDDEN);
}
void muse_conversation_ui_tick(bool visible,muse_mode_t mode) {
    if(!s_entry)return;
    s_allowed=visible&&mode==MUSE_MODE_IDLE&&!muse_state_asleep();
    lv_obj_set_flag(s_entry,LV_OBJ_FLAG_HIDDEN,!s_allowed||!muse_launcher_entries_visible());
    if(!s_allowed&&s_open)close_recent(NULL);
}
#endif
