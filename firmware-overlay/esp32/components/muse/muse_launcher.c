/* Public-edition staging: local UI additions and/or privacy review; see source manifest. */
/* SPDX-License-Identifier: Apache-2.0 */
#include "muse_launcher.h"
#if CONFIG_MUSE_BOARD_WAVESHARE_S3_175C
#include <math.h>
static lv_obj_t *entry,*overlay,*keys[5],*modals[8],*balance;
static unsigned modal_count;
static float progress,last_time=-1,painted=-1;
static bool wanted,available;
static const int centers[5][2]={{91,117},{56,184},{56,250},{70,312},{105,365}};
static void rect(lv_layer_t *l,int x,int y,int w,int h,uint32_t color,int opacity){lv_draw_rect_dsc_t d;lv_draw_rect_dsc_init(&d);d.bg_color=lv_color_hex(color);d.bg_opa=opacity;lv_area_t a={x,y,x+w-1,y+h-1};lv_draw_rect(l,&d,&a);}
static void draw_panel(lv_event_t *e){
 lv_event_stop_processing(e);lv_area_t at;lv_obj_get_coords(overlay,&at);int opacity=(int)(progress*225);if(!opacity)return;
 for(int y=92;y<396;y+=4){int dy=y-233;int x=233-(int)sqrtf(214.f*214-dy*dy)+4;x=x/4*4;rect(lv_event_get_layer(e),at.x1+x,at.y1+y,148-x,4,0x171322,opacity);rect(lv_event_get_layer(e),at.x1+x,at.y1+y,2,4,0x74618F,opacity);rect(lv_event_get_layer(e),at.x1+146,at.y1+y,2,4,0x74618F,opacity);}
}
static void draw_entry(lv_event_t *e){
 lv_event_stop_processing(e);lv_area_t at;lv_obj_get_coords(entry,&at);int x=at.x1+11,y=at.y1+11;lv_layer_t *l=lv_event_get_layer(e);
 if(wanted){for(int i=0;i<7;i++){rect(l,x+5+i*3,y+5+i*3,3,3,0xE8DCF8,255);rect(l,x+23-i*3,y+5+i*3,3,3,0xE8DCF8,255);}}
 else for(int a=0;a<3;a++)for(int b=0;b<3;b++)rect(l,x+5+a*8,y+5+b*8,5,5,(a+b)%2?0xB58AF5:0xEEE5FC,255);
 /* PREPROCESS replaces only the glyph via DRAW_POST; button skin stays. */
}
void muse_launcher_close(void){wanted=false;}
static void outside(lv_event_t *e){lv_event_stop_bubbling(e);muse_launcher_close();}
static void toggle(lv_event_t *e){(void)e;if(!available)return;muse_state_poke();wanted=!wanted;}
static void selected(lv_event_t *e){(void)e;muse_launcher_close();}
static bool modal_open(void){for(unsigned i=0;i<modal_count;i++)if(!lv_obj_has_flag(modals[i],LV_OBJ_FLAG_HIDDEN))return true;return false;}
void muse_launcher_build(lv_obj_t *face){
 overlay=lv_obj_create(face);lv_obj_remove_style_all(overlay);lv_obj_set_size(overlay,466,466);lv_obj_set_pos(overlay,0,0);lv_obj_remove_flag(overlay,LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_GESTURE_BUBBLE);lv_obj_add_flag(overlay,LV_OBJ_FLAG_HIDDEN|LV_OBJ_FLAG_CLICKABLE);lv_obj_add_event_cb(overlay,draw_panel,LV_EVENT_DRAW_MAIN|LV_EVENT_PREPROCESS,NULL);lv_obj_add_event_cb(overlay,outside,LV_EVENT_CLICKED,NULL);
 entry=lv_button_create(face);lv_obj_set_size(entry,54,54);lv_obj_set_pos(entry,26,208);lv_obj_set_style_radius(entry,LV_RADIUS_CIRCLE,0);lv_obj_set_style_bg_color(entry,lv_color_hex(0x302343),0);lv_obj_set_style_bg_color(entry,lv_color_hex(0x55416C),LV_STATE_PRESSED);lv_obj_set_style_border_width(entry,2,0);lv_obj_set_style_border_color(entry,lv_color_hex(0xB595DA),0);lv_obj_add_event_cb(entry,draw_entry,LV_EVENT_DRAW_POST,NULL);lv_obj_add_event_cb(entry,toggle,LV_EVENT_CLICKED,NULL);
}
void muse_launcher_add(unsigned id,lv_obj_t *button){if(id>=5||!button)return;keys[id]=button;lv_obj_set_align(button,LV_ALIGN_TOP_LEFT);lv_obj_add_event_cb(button,selected,LV_EVENT_CLICKED,NULL);}
void muse_launcher_modal(lv_obj_t *modal){if(modal&&modal_count<8)modals[modal_count++]=modal;}
void muse_launcher_balance(lv_obj_t *label){balance=label;}
bool muse_launcher_entries_visible(void){return available&&progress>0;}
void muse_launcher_before(bool visible,muse_mode_t mode,float now){
 available=visible&&mode==MUSE_MODE_IDLE&&!muse_state_asleep()&&!modal_open();
 if(!available){wanted=false;progress=0;}
 float dt=last_time<0?0:now-last_time;last_time=now;if(dt<0)dt=0;if(dt>.1f)dt=.1f;
 if(available){progress+= (wanted?1:-1)*dt/.22f;if(progress<0)progress=0;if(progress>1)progress=1;}
}
void muse_launcher_after(void){
 if(!entry)return;
 if(modal_open()){wanted=false;available=false;progress=0;}
 lv_obj_set_flag(entry,LV_OBJ_FLAG_HIDDEN,!available);lv_obj_set_flag(overlay,LV_OBJ_FLAG_HIDDEN,!available||progress<=0);
 bool moving=progress>0&&progress<1;
 for(int i=0;i<5;i++)if(keys[i]){lv_obj_set_flag(keys[i],LV_OBJ_FLAG_HIDDEN,!muse_launcher_entries_visible());lv_obj_set_flag(keys[i],LV_OBJ_FLAG_CLICKABLE,available&&wanted&&!moving);}
 if(balance)lv_obj_set_flag(balance,LV_OBJ_FLAG_HIDDEN,!available||progress<1||!wanted);
 if(painted!=progress){
  painted=progress;lv_obj_set_pos(entry,26+(int)(64*progress),208);
  for(int i=0;i<5;i++)if(keys[i]){lv_obj_set_pos(keys[i],53+(int)((centers[i][0]-53)*progress)-23,235+(int)((centers[i][1]-235)*progress)-23);lv_obj_set_style_opa(keys[i],(lv_opa_t)(255*progress),0);}
  lv_obj_invalidate(entry);lv_obj_invalidate(overlay);
 }
}
#endif
