#include "muse_i18n.h"
/* Public-edition staging: local UI additions and/or privacy review; see source manifest. */
/* SPDX-License-Identifier: Apache-2.0 */
#include "muse_style_ui.h"
#if CONFIG_MUSE_BOARD_WAVESHARE_S3_175C
#include "muse_style.h"
#include "muse_launcher.h"
#include "muse_pixel.h"
#include "muse_ui_fonts.h"
#include <stdlib.h>
#include <stdatomic.h>
#include <stdio.h>
#if defined(ESP_PLATFORM) && CONFIG_SPIRAM
#include "esp_attr.h"
#define STYLE_BIG EXT_RAM_BSS_ATTR
#else
#define STYLE_BIG
#endif
#include "src/misc/cache/instance/lv_image_cache.h"
static lv_obj_t *ws_grid;
static lv_obj_t *ws_entry,*ws_modal,*ws_panel,*ws_tabs[3],*ws_choices[6],*ws_checks[6],*ws_note,*ws_preview,*ws_bg,*ws_canvas;
static lv_image_dsc_t ws_scene_image;
static STYLE_BIG uint16_t ws_scene_data[116*116],ws_preview_data[64*64];
static lv_image_dsc_t ws_preview_image;
static muse_style_t ws_draft,ws_confirmed;
static bool ws_allowed,ws_opened,ws_pending;
static unsigned ws_category,ws_last_scene=99;
static atomic_uint ws_request;
void muse_style_ui_request(unsigned action,unsigned value){if(action>=1&&action<=5&&value<6)atomic_store(&ws_request,(action<<8)|value);}
static const char *ws_titles[]={MUSE_KEY_s181,MUSE_KEY_s179,MUSE_KEY_s185};
static const char *ws_names[3][6]={
 {MUSE_KEY_s186,MUSE_KEY_s164,MUSE_KEY_s178,MUSE_KEY_s172,MUSE_KEY_s175,MUSE_KEY_s167},
 {MUSE_KEY_s163,MUSE_KEY_s183,MUSE_KEY_s169},{MUSE_KEY_s180,MUSE_KEY_s168,MUSE_KEY_s177,MUSE_KEY_s170}};
static const unsigned ws_counts[]={6,3,4};
static const unsigned ws_background_order[]={0,1,2,3,4,5};
static unsigned ws_option_id(unsigned i){return ws_category?i:ws_background_order[i];}
static unsigned ws_selected(void){return ws_category==0?ws_draft.background:ws_category==1?ws_draft.clothing:ws_category==2?ws_draft.accessory:ws_draft.sunglasses;}
static void ws_set_selected(unsigned n){
 if(ws_category==0)ws_draft.background=n;
 else if(ws_category==1)ws_draft.clothing=n;
 else if(n==3)ws_draft.sunglasses=!ws_draft.sunglasses;
 else ws_draft.accessory=n;
}
static lv_obj_t *ws_label(lv_obj_t *parent,const char *text){lv_obj_t *o=lv_label_create(parent);lv_obj_set_style_text_font(o,muse_ui_font(&lv_font_montserrat_16),0);lv_obj_set_style_text_color(o,lv_color_hex(0xF4F0E7),0);lv_label_set_text(o,text);lv_obj_remove_flag(o,LV_OBJ_FLAG_CLICKABLE);return o;}
static void ws_refresh(void){
 for(unsigned i=0;i<3;i++)lv_obj_set_style_bg_color(ws_tabs[i],lv_color_hex(ws_category==i?0x536A63:0x293B38),0);
 for(unsigned i=0;i<6;i++){
  bool shown=i<ws_counts[ws_category];lv_obj_set_flag(ws_choices[i],LV_OBJ_FLAG_HIDDEN,!shown);
  if(!shown)continue;
  bool glasses=ws_category==2&&i==3;
  bool selected=glasses?ws_draft.sunglasses:ws_option_id(i)==ws_selected();
  lv_obj_t *text=lv_obj_get_child(ws_choices[i],0);
  lv_label_set_text(text,muse_i18n_text(ws_names[ws_category][ws_option_id(i)]));
  lv_obj_set_width(text,glasses?128:146);lv_obj_align(text,LV_ALIGN_LEFT_MID,4,0);
  lv_obj_set_flag(ws_checks[i],LV_OBJ_FLAG_HIDDEN,!glasses);
  lv_label_set_text(ws_checks[i],selected?LV_SYMBOL_OK:"");
  lv_obj_set_style_bg_color(ws_choices[i],lv_color_hex(selected?0x617769:0x30423E),0);
 }
 lv_label_set_text(ws_note,ws_pending?muse_i18n_text(MUSE_KEY_s165):ws_category==0?muse_i18n_text(MUSE_KEY_s182):ws_category==2?muse_i18n_text(MUSE_KEY_s322):muse_i18n_text(MUSE_KEY_s184));muse_pixel_style_preview(ws_preview_data,ws_draft.background,ws_draft.clothing,ws_draft.accessory,ws_draft.sunglasses);lv_image_cache_drop(&ws_preview_image);lv_obj_invalidate(ws_preview);
}
static void ws_close_style(lv_event_t *e){(void)e;if(ws_pending)return;ws_opened=false;ws_draft=ws_confirmed;lv_obj_add_flag(ws_modal,LV_OBJ_FLAG_HIDDEN);}
static void ws_apply_style(lv_event_t *e){(void)e;if(ws_pending)return;if(muse_style_save(&ws_draft)){ws_pending=true;ws_refresh();}else lv_label_set_text(ws_note,muse_i18n_text(MUSE_KEY_s173));}
static void ws_on_choice(lv_event_t *e){if(ws_pending)return;ws_set_selected(ws_option_id((unsigned)(uintptr_t)lv_event_get_user_data(e)));ws_refresh();}
static void ws_on_tab(lv_event_t *e){ws_category=(unsigned)(uintptr_t)lv_event_get_user_data(e);ws_refresh();lv_obj_scroll_to_y(ws_grid,0,LV_ANIM_OFF);}
static void ws_open_style(lv_event_t *e){(void)e;if(!ws_allowed||ws_pending||!muse_style_snapshot(&ws_confirmed))return;muse_state_poke();ws_draft=ws_confirmed;ws_opened=true;ws_category=0;ws_refresh();lv_obj_scroll_to_y(ws_grid,0,LV_ANIM_OFF);lv_obj_remove_flag(ws_modal,LV_OBJ_FLAG_HIDDEN);lv_obj_move_foreground(ws_modal);}
static void ws_draw_hanger(lv_event_t *e){
 lv_event_stop_processing(e);lv_area_t b;lv_obj_get_coords(lv_event_get_target_obj(e),&b);lv_draw_rect_dsc_t d;lv_draw_rect_dsc_init(&d);d.bg_opa=255;d.bg_color=lv_color_hex(0xE3CEAA);
 const uint16_t bits[12]={0x0380,0x0440,0x0040,0x0080,0x0100,0x0380,0x0C60,0x3018,0x4004,0x4004,0x7FFC,0};
 for(int y=0;y<12;y++)for(int x=0;x<15;x++)if(bits[y]&(1u<<(14-x))){lv_area_t a={b.x1+2*x,b.y1+2*y+4,b.x1+2*x+1,b.y1+2*y+5};lv_draw_rect(lv_event_get_layer(e),&d,&a);}
}
void muse_style_ui_build(lv_obj_t *face,lv_obj_t *avatar){
 ws_canvas=avatar;
 /* Background below every control and avatar; no full-screen RGBA buffer. */
 ws_bg=lv_image_create(face);ws_scene_image.header.magic=LV_IMAGE_HEADER_MAGIC;ws_scene_image.header.cf=LV_COLOR_FORMAT_RGB565;ws_scene_image.header.w=116;ws_scene_image.header.h=116;ws_scene_image.header.stride=232;ws_scene_image.data_size=sizeof ws_scene_data;ws_scene_image.data=(const uint8_t*)ws_scene_data;lv_image_set_src(ws_bg,&ws_scene_image);lv_image_set_scale(ws_bg,1024);lv_image_set_antialias(ws_bg,false);lv_obj_center(ws_bg);lv_obj_move_background(ws_bg);lv_obj_remove_flag(ws_bg,LV_OBJ_FLAG_CLICKABLE);lv_obj_add_flag(ws_bg,LV_OBJ_FLAG_HIDDEN);
 ws_entry=lv_button_create(face);lv_obj_set_size(ws_entry,46,46);lv_obj_align(ws_entry,LV_ALIGN_CENTER,-163,79);lv_obj_set_style_radius(ws_entry,LV_RADIUS_CIRCLE,0);lv_obj_set_style_bg_color(ws_entry,lv_color_hex(0x222734),0);lv_obj_set_style_border_width(ws_entry,1,0);lv_obj_set_style_border_color(ws_entry,lv_color_hex(0x596176),0);
 lv_obj_t *ic=lv_obj_create(ws_entry);lv_obj_remove_style_all(ic);lv_obj_set_size(ic,32,32);lv_obj_center(ic);lv_obj_remove_flag(ic,LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);lv_obj_add_event_cb(ic,ws_draw_hanger,LV_EVENT_DRAW_MAIN|LV_EVENT_PREPROCESS,NULL);lv_obj_add_event_cb(ws_entry,ws_open_style,LV_EVENT_CLICKED,NULL);
 muse_launcher_add(3,ws_entry);
 ws_modal=lv_obj_create(lv_layer_top());lv_obj_remove_style_all(ws_modal);lv_obj_set_size(ws_modal,lv_pct(100),lv_pct(100));lv_obj_set_style_bg_color(ws_modal,lv_color_black(),0);lv_obj_set_style_bg_opa(ws_modal,LV_OPA_80,0);lv_obj_add_flag(ws_modal,LV_OBJ_FLAG_HIDDEN|LV_OBJ_FLAG_CLICKABLE);lv_obj_remove_flag(ws_modal,LV_OBJ_FLAG_SCROLLABLE);lv_obj_add_event_cb(ws_modal,ws_close_style,LV_EVENT_CLICKED,NULL);
 muse_launcher_modal(ws_modal);
 ws_panel=lv_obj_create(ws_modal);lv_obj_remove_style_all(ws_panel);lv_obj_set_size(ws_panel,352,374);lv_obj_center(ws_panel);lv_obj_set_style_bg_color(ws_panel,lv_color_hex(0x1D302C),0);lv_obj_set_style_bg_opa(ws_panel,255,0);lv_obj_set_style_border_width(ws_panel,2,0);lv_obj_set_style_border_color(ws_panel,lv_color_hex(0xC4B48B),0);lv_obj_set_style_radius(ws_panel,20,0);lv_obj_add_flag(ws_panel,LV_OBJ_FLAG_CLICKABLE);lv_obj_remove_flag(ws_panel,LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_EVENT_BUBBLE);
 lv_obj_t *title=ws_label(ws_panel,muse_i18n_text(MUSE_KEY_s176));lv_obj_align(title,LV_ALIGN_TOP_MID,0,12);
 ws_preview=lv_image_create(ws_panel);ws_preview_image.header.magic=LV_IMAGE_HEADER_MAGIC;ws_preview_image.header.cf=LV_COLOR_FORMAT_RGB565;ws_preview_image.header.w=64;ws_preview_image.header.h=64;ws_preview_image.header.stride=128;ws_preview_image.data_size=sizeof ws_preview_data;ws_preview_image.data=(const uint8_t*)ws_preview_data;lv_image_set_src(ws_preview,&ws_preview_image);lv_image_set_scale(ws_preview,512);lv_image_set_antialias(ws_preview,false);lv_obj_align(ws_preview,LV_ALIGN_TOP_MID,0,68);lv_obj_remove_flag(ws_preview,LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);

 for(unsigned i=0;i<3;i++){ws_tabs[i]=lv_button_create(ws_panel);lv_obj_set_size(ws_tabs[i],104,36);lv_obj_set_pos(ws_tabs[i],12+i*112,164);lv_obj_set_style_pad_all(ws_tabs[i],0,0);lv_obj_t *l=ws_label(ws_tabs[i],muse_i18n_text(ws_titles[i]));lv_obj_set_width(l,96);lv_obj_set_style_text_align(l,LV_TEXT_ALIGN_CENTER,0);lv_obj_center(l);lv_obj_add_event_cb(ws_tabs[i],ws_on_tab,LV_EVENT_CLICKED,(void*)(uintptr_t)i);}
 ws_grid=lv_obj_create(ws_panel);lv_obj_remove_style_all(ws_grid);lv_obj_set_size(ws_grid,330,104);lv_obj_set_pos(ws_grid,11,207);lv_obj_set_scroll_dir(ws_grid,LV_DIR_VER);lv_obj_set_scrollbar_mode(ws_grid,LV_SCROLLBAR_MODE_AUTO);lv_obj_set_style_pad_all(ws_grid,0,0);
 for(unsigned i=0;i<6;i++){ws_choices[i]=lv_button_create(ws_grid);lv_obj_set_size(ws_choices[i],156,44);lv_obj_set_pos(ws_choices[i],4+(i%2)*165,(i/2)*52);lv_obj_set_style_pad_all(ws_choices[i],0,0);lv_obj_t *l=ws_label(ws_choices[i],"");lv_obj_set_style_text_align(l,LV_TEXT_ALIGN_CENTER,0);lv_obj_set_width(l,146);lv_label_set_long_mode(l,LV_LABEL_LONG_MODE_WRAP);lv_obj_align(l,LV_ALIGN_LEFT_MID,4,0);ws_checks[i]=ws_label(ws_choices[i],"");lv_obj_set_size(ws_checks[i],16,22);lv_obj_set_style_border_width(ws_checks[i],1,0);lv_obj_set_style_border_color(ws_checks[i],lv_color_hex(0xC4B48B),0);lv_obj_set_style_radius(ws_checks[i],2,0);lv_obj_set_style_text_align(ws_checks[i],LV_TEXT_ALIGN_CENTER,0);lv_obj_align(ws_checks[i],LV_ALIGN_RIGHT_MID,-3,0);lv_obj_add_flag(ws_checks[i],LV_OBJ_FLAG_HIDDEN);lv_obj_add_event_cb(ws_choices[i],ws_on_choice,LV_EVENT_CLICKED,(void*)(uintptr_t)i);}
 ws_note=ws_label(ws_panel,"");lv_obj_set_width(ws_note,330);lv_obj_set_style_text_align(ws_note,LV_TEXT_ALIGN_CENTER,0);lv_obj_set_pos(ws_note,11,316);
 lv_obj_t *cancel=lv_button_create(ws_panel);lv_obj_set_size(cancel,90,30);lv_obj_set_pos(cancel,48,340);lv_obj_t *l=ws_label(cancel,muse_i18n_text(MUSE_KEY_s068));lv_obj_center(l);lv_obj_add_event_cb(cancel,ws_close_style,LV_EVENT_CLICKED,NULL);
 lv_obj_t *apply=lv_button_create(ws_panel);lv_obj_set_size(apply,90,30);lv_obj_set_pos(apply,210,340);l=ws_label(apply,muse_i18n_text(MUSE_KEY_s171));lv_obj_center(l);lv_obj_add_event_cb(apply,ws_apply_style,LV_EVENT_CLICKED,NULL);
}
void muse_style_ui_tick(bool visible,muse_mode_t mode){
 if(!ws_entry)return;
 ws_allowed=visible&&mode==MUSE_MODE_IDLE&&!muse_state_asleep();lv_obj_set_flag(ws_entry,LV_OBJ_FLAG_HIDDEN,!ws_allowed||!muse_launcher_entries_visible());
 unsigned command=atomic_exchange(&ws_request,0),action=command>>8,value=command&255;
 if(action==1)ws_open_style(NULL);
 if(ws_opened&&!ws_pending){
  if(action==2&&value<3){ws_category=value;ws_refresh();lv_obj_scroll_to_y(ws_grid,0,LV_ANIM_OFF);}
  if(action==3&&value<ws_counts[ws_category]){ws_set_selected(value);ws_refresh();}
  if(action==4)ws_apply_style(NULL);
  if(action==5)ws_close_style(NULL);
 }
 if(command){printf("@style.ui open=%d category=%u selected=%u pending=%d\n",ws_opened,ws_category,ws_selected(),ws_pending);fflush(stdout);}
 int result=muse_style_feedback();if(result){ws_pending=false;if(result>0){muse_style_snapshot(&ws_confirmed);ws_draft=ws_confirmed;if(ws_opened){ws_opened=false;lv_obj_add_flag(ws_modal,LV_OBJ_FLAG_HIDDEN);}}else{ws_draft=ws_confirmed;if(ws_opened){ws_refresh();lv_label_set_text(ws_note,muse_i18n_text(MUSE_KEY_s166));}}}
 if(!ws_allowed&&ws_opened){ws_opened=false;lv_obj_add_flag(ws_modal,LV_OBJ_FLAG_HIDDEN);ws_draft=ws_confirmed;}
 if(!ws_opened)muse_style_snapshot(&ws_confirmed);
 muse_style_t s=ws_opened?ws_draft:ws_confirmed;muse_pixel_set_style(s.clothing,s.accessory,s.sunglasses);
 unsigned scene=ws_allowed?s.background:0;
 if(ws_last_scene!=scene){ws_last_scene=scene;for(int y=0;y<116;y++)for(int x=0;x<116;x++)ws_scene_data[y*116+x]=muse_pixel_scene_pixel(scene,x*4+1,y*4+1);lv_image_cache_drop(&ws_scene_image);lv_obj_invalidate(ws_bg);lv_obj_invalidate(ws_canvas);}
 lv_obj_set_flag(ws_bg,LV_OBJ_FLAG_HIDDEN,scene==0);
 lv_area_t at;lv_obj_get_coords(ws_canvas,&at);muse_pixel_set_scene(scene,at.x1,at.y1);muse_pixel_set_size(lv_obj_get_width(ws_canvas));
}
#endif
