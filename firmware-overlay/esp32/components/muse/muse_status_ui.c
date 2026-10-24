#include "muse_i18n.h"
/* Public-edition staging: local UI additions and/or privacy review; see source manifest. */
#include "muse_status_ui.h"
#if CONFIG_MUSE_BOARD_WAVESHARE_S3_175C
#include "muse_launcher.h"
#include "muse_ui_fonts.h"
#include <stdio.h>
#include <string.h>
static lv_obj_t *status_button,*wifi_icon,*percent,*modal,*detail_label;
static muse_power_t battery={.battery_pct=-1};
static uint32_t status_color=0x8C8993;
static bool connected,allowed;
static char detail[192];
static void box(lv_layer_t *l,int x,int y,int w,int h,uint32_t color,int opacity){if(w<=0||h<=0)return;lv_draw_rect_dsc_t d;lv_draw_rect_dsc_init(&d);d.bg_opa=opacity;d.bg_color=lv_color_hex(color);lv_area_t a={x,y,x+w-1,y+h-1};lv_draw_rect(l,&d,&a);}
static void draw(lv_event_t *e){lv_event_stop_processing(e);lv_area_t a;lv_obj_get_coords(status_button,&a);int x=a.x1,y=a.y1+7;lv_layer_t *l=lv_event_get_layer(e);uint32_t bg=lv_obj_has_state(status_button,LV_STATE_PRESSED)?0x382B48:0x121923;box(l,x+4,y,124,30,bg,235);box(l,x,y+4,132,22,bg,235);uint32_t edge=battery.charging?0x8BDDDC:0xD9E0DA;box(l,x+55,y+4,51,22,edge,255);box(l,x+57,y+6,47,18,0x15231E,255);int value=battery.battery_pct;if(value>100)value=100;if(value<0)value=0;box(l,x+59,y+8,43*value/100,14,value<=15?0x813D49:value<=30?0x77592D:0x365B49,255);box(l,x+106,y+10,3,10,edge,255);if(!connected)for(int i=0;i<8;i++)box(l,x+17+i*2,y+7+i*2,2,2,0xA79BAD,255);lv_draw_rect_dsc_t d;lv_draw_rect_dsc_init(&d);d.bg_opa=255;d.bg_color=lv_color_hex(status_color);d.radius=LV_RADIUS_CIRCLE;d.border_width=1;d.border_color=lv_color_hex(0xC2B8CF);lv_area_t dot={x+117,y+10,x+126,y+19};lv_draw_rect(l,&d,&dot);}
static lv_obj_t *label(lv_obj_t *parent,const char *text,const lv_font_t *font){lv_obj_t *o=lv_label_create(parent);lv_obj_set_style_text_font(o,font,0);lv_obj_set_style_text_color(o,lv_color_hex(0xF0EDF5),0);lv_label_set_text(o,text);lv_obj_remove_flag(o,LV_OBJ_FLAG_CLICKABLE);return o;}
static void close_info(lv_event_t *e){(void)e;lv_obj_add_flag(modal,LV_OBJ_FLAG_HIDDEN);}
static void open_info(lv_event_t *e){(void)e;if(!allowed)return;muse_state_poke();muse_launcher_close();lv_label_set_text(detail_label,detail);lv_obj_remove_flag(modal,LV_OBJ_FLAG_HIDDEN);lv_obj_move_foreground(modal);}
void muse_status_ui_build(lv_obj_t *face){
 status_button=lv_button_create(face);lv_obj_remove_style_all(status_button);lv_obj_set_size(status_button,132,44);lv_obj_set_pos(status_button,167,13);lv_obj_remove_flag(status_button,LV_OBJ_FLAG_SCROLLABLE);lv_obj_add_event_cb(status_button,draw,LV_EVENT_DRAW_MAIN|LV_EVENT_PREPROCESS,NULL);lv_obj_add_event_cb(status_button,open_info,LV_EVENT_CLICKED,NULL);
 wifi_icon=label(status_button,LV_SYMBOL_WIFI,&lv_font_montserrat_16);lv_obj_set_pos(wifi_icon,15,11);percent=label(status_button,"--",&lv_font_montserrat_14);lv_obj_set_size(percent,45,18);lv_obj_set_pos(percent,58,13);lv_obj_set_style_text_align(percent,LV_TEXT_ALIGN_CENTER,0);
 modal=lv_obj_create(lv_layer_top());lv_obj_remove_style_all(modal);lv_obj_set_size(modal,lv_pct(100),lv_pct(100));lv_obj_set_style_bg_color(modal,lv_color_black(),0);lv_obj_set_style_bg_opa(modal,LV_OPA_80,0);lv_obj_add_flag(modal,LV_OBJ_FLAG_HIDDEN|LV_OBJ_FLAG_CLICKABLE);lv_obj_remove_flag(modal,LV_OBJ_FLAG_SCROLLABLE);lv_obj_add_event_cb(modal,close_info,LV_EVENT_CLICKED,NULL);muse_launcher_modal(modal);
 lv_obj_t *panel=lv_obj_create(modal);lv_obj_set_size(panel,310,240);lv_obj_center(panel);lv_obj_set_style_bg_color(panel,lv_color_hex(0x211B2F),0);lv_obj_set_style_border_color(panel,lv_color_hex(0xA995C6),0);lv_obj_set_style_border_width(panel,2,0);lv_obj_set_style_radius(panel,20,0);lv_obj_remove_flag(panel,LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_EVENT_BUBBLE);
 lv_obj_t *title=label(panel,muse_i18n_text(MUSE_KEY_s161),muse_ui_font(&lv_font_montserrat_16));lv_obj_align(title,LV_ALIGN_TOP_MID,0,0);detail_label=label(panel,"",muse_ui_font(&lv_font_montserrat_16));lv_obj_set_width(detail_label,264);lv_obj_set_pos(detail_label,4,42);lv_obj_set_style_text_line_space(detail_label,8,0);
 lv_obj_t *done=lv_button_create(panel);lv_obj_set_size(done,90,36);lv_obj_align(done,LV_ALIGN_BOTTOM_MID,0,0);lv_obj_add_event_cb(done,close_info,LV_EVENT_CLICKED,NULL);lv_obj_t *text=label(done,muse_i18n_text(MUSE_KEY_s096),muse_ui_font(&lv_font_montserrat_16));lv_obj_center(text);
}
void muse_status_ui_update(muse_power_t p,bool wifi,uint32_t color,const char *text){if(!status_button)return;if(p.battery_pct!=battery.battery_pct||p.charging!=battery.charging||p.usb!=battery.usb||wifi!=connected||color!=status_color){battery=p;connected=wifi;status_color=color;char value[12];if(p.battery_pct<0)snprintf(value,sizeof value,"%s",p.usb?"USB":"--");else snprintf(value,sizeof value,"%d%%",p.battery_pct>100?100:p.battery_pct);lv_label_set_text(percent,value);lv_obj_set_style_text_color(wifi_icon,lv_color_hex(wifi?0xE5E1F4:0x93879F),0);lv_obj_invalidate(status_button);}if(text&&strcmp(text,detail)){snprintf(detail,sizeof detail,"%s",text);if(!lv_obj_has_flag(modal,LV_OBJ_FLAG_HIDDEN))lv_label_set_text(detail_label,detail);}}
void muse_status_ui_tick(bool visible,muse_mode_t mode){if(!status_button)return;allowed=visible&&mode==MUSE_MODE_IDLE&&!muse_state_asleep();lv_obj_set_flag(status_button,LV_OBJ_FLAG_HIDDEN,!visible||muse_state_asleep());if(!allowed)close_info(NULL);}
#endif
