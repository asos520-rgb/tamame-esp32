#include "muse_i18n.h"
/* Public-edition staging: local UI additions and/or privacy review; see source manifest. */
/* SPDX-License-Identifier: Apache-2.0 */
#include "muse_pet_ui.h"
#if CONFIG_MUSE_BOARD_WAVESHARE_S3_175C
#include "muse_pet.h"
#include "muse_launcher.h"
#include "muse_hint_ui.h"
static muse_hint_panel_t s_hint;
#include "muse_pet_anim.h"
#include "muse_pet_art.h"
#include "muse_voice.h"
#include "muse_ui_fonts.h"
#include "esp_timer.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
static lv_obj_t *s_canvas,*s_hud,*s_coin,*s_feed,*s_fortune,*s_food,*s_flying,*s_toast;
static lv_obj_t *s_modal,*s_panel,*s_title,*s_body,*s_group[3],*s_choices[6],*s_bars[3],*s_values[3],*s_wallet,*s_grade;
static const muse_pet_action_t actions[]={PET_COFFEE,PET_CAKE,PET_RICE,PET_HOTPOT,PET_CHICKEN,PET_TEA};
static const uint32_t colors[]={0xDDAF63,0xDB8F9D,0x7BBBC9};
static const lv_font_t *s_font;
static bool s_allowed,s_touch_allowed,s_pending,s_held,s_long,s_open,s_coin_landed;
static uint32_t s_press_ms;
static lv_point_t s_press;
static float s_now,s_anim_at,s_toast_until,s_flip_until,s_coin_at=-100;
static muse_pet_anim_kind_t s_kind;
static muse_pet_anim_frame_t s_anim;
static int s_offset,s_page;
static uint32_t s_last_coins=UINT32_MAX;
static int64_t s_last_chew=-1;
/* Only the custom 175C renderer implements these independent idle expressions. */
extern void muse_pixel_set_interaction(int state);
static lv_obj_t *label(lv_obj_t *parent,const char *text) {
    lv_obj_t *o=lv_label_create(parent);lv_obj_set_style_text_font(o,s_font,0);
    lv_obj_set_style_text_color(o,lv_color_hex(0xF4F0E7),0);lv_label_set_text(o,text);
    lv_obj_remove_flag(o,LV_OBJ_FLAG_CLICKABLE);return o;
}
static void toast(const char *text) {
    lv_label_set_text(s_toast,text);lv_obj_remove_flag(s_toast,LV_OBJ_FLAG_HIDDEN);s_toast_until=s_now+4;muse_hint_panel_sync(&s_hint,true);
}
static void draw_icon(lv_event_t *e) {
    lv_event_stop_processing(e);lv_obj_t *o=lv_event_get_target_obj(e);lv_area_t box;lv_obj_get_coords(o,&box);
    int icon=(int)(intptr_t)lv_event_get_user_data(e),scale=lv_obj_get_width(o)/16;
    if(o==s_food)icon=s_kind==PET_ANIM_COFFEE?3:s_kind==PET_ANIM_CAKE?4:s_kind==PET_ANIM_RICE?5:s_kind==PET_ANIM_HOTPOT?6:s_kind==PET_ANIM_CHICKEN?7:8;
    lv_draw_rect_dsc_t d;lv_draw_rect_dsc_init(&d);d.radius=0;d.border_width=0;d.bg_opa=LV_OPA_COVER;
    for(int y=0;y<16;y++)for(int x=0;x<16;x++) {
        uint32_t c=muse_pet_art_pixel((unsigned)icon,x,y);if(!c)continue;
        d.bg_color=lv_color_hex(c);lv_area_t a={box.x1+x*scale,box.y1+y*scale,box.x1+(x+1)*scale-1,box.y1+(y+1)*scale-1};
        lv_draw_rect(lv_event_get_layer(e),&d,&a);
    }
}
static lv_obj_t *icon(lv_obj_t *parent,unsigned id,int size) {
    lv_obj_t *o=lv_obj_create(parent);lv_obj_remove_style_all(o);lv_obj_set_size(o,size,size);
    lv_obj_remove_flag(o,LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(o,draw_icon,LV_EVENT_DRAW_MAIN|LV_EVENT_PREPROCESS,(void*)(intptr_t)id);return o;
}
static void close_modal(lv_event_t *e) {
    (void)e;s_open=false;s_flip_until=0;lv_obj_set_width(s_panel,334);lv_obj_add_flag(s_modal,LV_OBJ_FLAG_HIDDEN);
}
static void show_page(int page,const char *title) {
    static const uint32_t bg[]={0x152A37,0x1D2B21,0x282538},edge[]={0x80B5C8,0xB4BF87,0xC7AF86};
    s_page=page;s_open=true;lv_label_set_text(s_title,title);
    for(int i=0;i<3;i++)lv_obj_set_flag(s_group[i],LV_OBJ_FLAG_HIDDEN,i!=page);
    lv_obj_set_style_bg_color(s_panel,lv_color_hex(bg[page]),0);lv_obj_set_style_border_color(s_panel,lv_color_hex(edge[page]),0);
    lv_obj_remove_flag(s_modal,LV_OBJ_FLAG_HIDDEN);lv_obj_move_foreground(s_modal);
}
static void update_card(const muse_pet_model_t *s) {
    const int values[]={s->hunger/1000,s->mood/1000,s->energy/1000};
    for(int i=0;i<3;i++){lv_bar_set_value(s_bars[i],values[i],LV_ANIM_OFF);lv_label_set_text_fmt(s_values[i],"%d / 100",values[i]);}
    lv_label_set_text_fmt(s_wallet,muse_i18n_text(MUSE_KEY_s048),(unsigned long)s->coins,(unsigned long)s->daily_pets);
}
static void status_card(void) {
    muse_pet_model_t s;if(!muse_pet_snapshot(&s)){toast(muse_i18n_text(MUSE_KEY_s047));return;}update_card(&s);show_page(0,muse_i18n_text(MUSE_KEY_s035));
}
static bool request(muse_pet_action_t action) {
    bool allowed=action==PET_TOUCH?s_touch_allowed:s_allowed;
    if(!allowed||s_pending||s_kind!=PET_ANIM_NONE){if(s_allowed)toast(muse_i18n_text(MUSE_KEY_s043));return false;}
    if(!muse_pet_request(action)){toast(muse_i18n_text(MUSE_KEY_s046));return false;}
    s_pending=true;return true;
}
static void on_coin(lv_event_t *e){(void)e;(void)request(PET_TOUCH);}
static void on_feed(lv_event_t *e) {
    (void)e;if(!s_allowed||s_pending||s_kind!=PET_ANIM_NONE){toast(muse_i18n_text(MUSE_KEY_s044));return;}show_page(1,muse_i18n_text(MUSE_KEY_s034));
}
static void on_food(lv_event_t *e) {
    muse_pet_action_t action=(muse_pet_action_t)(intptr_t)lv_event_get_user_data(e);if(request(action))close_modal(NULL);
}
static void on_fortune(lv_event_t *e){(void)e;(void)request(PET_FORTUNE);}
static void on_canvas(lv_event_t *e) {
    lv_indev_t *indev=lv_indev_active();if(!indev)return;
    lv_point_t p;lv_indev_get_point(indev,&p);uint32_t ms=(uint32_t)(esp_timer_get_time()/1000);
    lv_event_code_t code=lv_event_get_code(e);
    if(code==LV_EVENT_PRESSED){s_held=true;s_long=false;s_press=p;s_press_ms=ms;return;}
    if(code==LV_EVENT_PRESS_LOST){s_held=false;return;}
    if(!s_held)return;
    if(abs(p.x-s_press.x)>12||abs(p.y-s_press.y)>12||lv_indev_get_scroll_obj(indev)){s_held=false;return;}
    if(code==LV_EVENT_PRESSING&&!s_long&&ms-s_press_ms>=1000){s_long=true;if(s_allowed){muse_state_poke();status_card();}}
    if(code==LV_EVENT_RELEASED){s_held=false;if(s_long)return;if(ms-s_press_ms>=1000){if(s_allowed)status_card();return;}if(s_touch_allowed){muse_state_poke();(void)request(PET_TOUCH);}}
}
static lv_obj_t *round_button(lv_obj_t *parent,unsigned id,int x,int y,lv_event_cb_t cb) {
    lv_obj_t *o=lv_button_create(parent);lv_obj_set_size(o,46,46);lv_obj_align(o,LV_ALIGN_CENTER,x,y);
    lv_obj_set_style_bg_color(o,lv_color_hex(0x222734),0);lv_obj_set_style_radius(o,LV_RADIUS_CIRCLE,0);
    lv_obj_set_style_border_width(o,1,0);lv_obj_set_style_border_color(o,lv_color_hex(0x596176),0);
    lv_obj_t *i=icon(o,id,32);lv_obj_center(i);lv_obj_add_event_cb(o,cb,LV_EVENT_CLICKED,NULL);return o;
}
static void draw_paper(lv_event_t *e) {
    lv_event_stop_processing(e);lv_area_t b;lv_obj_get_coords(lv_event_get_target_obj(e),&b);
    lv_draw_rect_dsc_t d;lv_draw_rect_dsc_init(&d);d.bg_color=lv_color_hex(0xF2E3BC);d.bg_opa=255;d.border_width=0;
    lv_area_t a={b.x1+5,b.y1,b.x2-5,b.y2};lv_draw_rect(lv_event_get_layer(e),&d,&a);
    a=(lv_area_t){b.x1,b.y1+8,b.x2,b.y2-8};lv_draw_rect(lv_event_get_layer(e),&d,&a);
    d.bg_color=lv_color_hex(0xCDA975);a=(lv_area_t){b.x1+10,b.y1+16,b.x2-10,b.y1+18};lv_draw_rect(lv_event_get_layer(e),&d,&a);
    a=(lv_area_t){b.x1+10,b.y2-18,b.x2-10,b.y2-16};lv_draw_rect(lv_event_get_layer(e),&d,&a);
}
void muse_pet_ui_build(lv_obj_t *face,lv_obj_t *canvas,const lv_font_t *font) {
    (void)font;s_font=muse_ui_font(&lv_font_montserrat_16);s_canvas=canvas;
    /* Three 46px keys along the left arc. Lower arc slots remain free for phase 2. */
    s_coin=round_button(face,0,-142,-116,on_coin);s_feed=round_button(face,1,-177,-49,on_feed);s_fortune=round_button(face,2,-177,17,on_fortune);
    s_hud=label(face,"--");lv_obj_set_size(s_hud,60,18);lv_obj_set_pos(s_hud,120,109);lv_obj_set_style_text_color(s_hud,lv_color_hex(0xF6CB4B),0);lv_obj_set_style_text_font(s_hud,&lv_font_montserrat_14,0);
    s_food=icon(face,3,48);lv_obj_add_flag(s_food,LV_OBJ_FLAG_HIDDEN);
    s_flying=icon(face,0,32);lv_obj_add_flag(s_flying,LV_OBJ_FLAG_HIDDEN);
    muse_hint_panel_create(&s_hint,face,s_font);s_toast=s_hint.label;lv_obj_add_flag(s_toast,LV_OBJ_FLAG_HIDDEN);
    muse_launcher_add(0,s_coin);muse_launcher_add(1,s_feed);muse_launcher_add(2,s_fortune);muse_launcher_balance(s_hud);
    s_modal=lv_obj_create(lv_layer_top());lv_obj_remove_style_all(s_modal);lv_obj_set_size(s_modal,lv_pct(100),lv_pct(100));
    lv_obj_set_style_bg_color(s_modal,lv_color_black(),0);lv_obj_set_style_bg_opa(s_modal,LV_OPA_80,0);
    lv_obj_add_flag(s_modal,LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_HIDDEN);lv_obj_remove_flag(s_modal,LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(s_modal,close_modal,LV_EVENT_CLICKED,NULL);
    muse_launcher_modal(s_modal);
    s_panel=lv_obj_create(s_modal);lv_obj_remove_style_all(s_panel);lv_obj_set_size(s_panel,334,354);lv_obj_center(s_panel);
    lv_obj_set_style_bg_opa(s_panel,LV_OPA_COVER,0);lv_obj_set_style_border_width(s_panel,2,0);lv_obj_set_style_radius(s_panel,22,0);
    lv_obj_remove_flag(s_panel,LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);
    s_title=label(s_panel,"");lv_obj_align(s_title,LV_ALIGN_TOP_MID,0,22);
    for(int i=0;i<3;i++){s_group[i]=lv_obj_create(s_panel);lv_obj_remove_style_all(s_group[i]);lv_obj_set_size(s_group[i],330,270);lv_obj_align(s_group[i],LV_ALIGN_TOP_MID,0,60);lv_obj_remove_flag(s_group[i],LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);}
    const char *names[]={muse_i18n_text(MUSE_KEY_s050),muse_i18n_text(MUSE_KEY_s040),muse_i18n_text(MUSE_KEY_s045)};
    for(int i=0;i<3;i++) {
        lv_obj_t *l=label(s_group[0],names[i]);lv_obj_set_pos(l,30,6+i*65);
        s_values[i]=label(s_group[0],"");lv_obj_set_style_text_font(s_values[i],&lv_font_montserrat_16,0);lv_obj_set_pos(s_values[i],210,6+i*65);
        s_bars[i]=lv_bar_create(s_group[0]);lv_obj_set_size(s_bars[i],270,16);lv_obj_set_pos(s_bars[i],30,32+i*65);
        lv_obj_set_style_bg_color(s_bars[i],lv_color_hex(0x344654),LV_PART_MAIN);lv_obj_set_style_bg_color(s_bars[i],lv_color_hex(colors[i]),LV_PART_INDICATOR);
        lv_obj_remove_flag(s_bars[i],LV_OBJ_FLAG_CLICKABLE);
    }
    s_wallet=label(s_group[0],"");lv_obj_align(s_wallet,LV_ALIGN_BOTTOM_MID,0,-36);lv_obj_set_style_text_align(s_wallet,LV_TEXT_ALIGN_CENTER,0);lv_obj_set_style_text_line_space(s_wallet,5,0);
    for(int i=0;i<6;i++) {
        const muse_pet_food_t *f=muse_pet_food(actions[i]);lv_obj_t *o=lv_button_create(s_group[1]);s_choices[i]=o;
        lv_obj_set_size(o,92,108);lv_obj_set_pos(o,17+(i%3)*102,4+(i/3)*118);lv_obj_set_style_radius(o,12,0);
        lv_obj_set_style_bg_color(o,lv_color_hex(0x304136),0);lv_obj_t *pic=icon(o,3+i,32);lv_obj_align(pic,LV_ALIGN_TOP_MID,0,-2);
        lv_obj_t *l=label(o,muse_i18n_text(f->name));lv_obj_align(l,LV_ALIGN_TOP_MID,0,38);l=label(o,"");lv_label_set_text_fmt(l,muse_i18n_text(MUSE_KEY_s032),f->cost);lv_obj_set_style_text_color(l,lv_color_hex(0xF6CB4B),0);lv_obj_align(l,LV_ALIGN_TOP_MID,0,61);
        lv_obj_add_event_cb(o,on_food,LV_EVENT_CLICKED,(void*)(intptr_t)actions[i]);
    }
    lv_obj_t *paper=lv_obj_create(s_group[2]);lv_obj_remove_style_all(paper);lv_obj_set_size(paper,78,212);lv_obj_set_pos(paper,34,3);
    lv_obj_remove_flag(paper,LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);lv_obj_add_event_cb(paper,draw_paper,LV_EVENT_DRAW_MAIN|LV_EVENT_PREPROCESS,NULL);
    s_grade=label(paper,"");lv_obj_set_width(s_grade,70);lv_obj_set_style_text_align(s_grade,LV_TEXT_ALIGN_CENTER,0);lv_label_set_long_mode(s_grade,LV_LABEL_LONG_MODE_WRAP);lv_obj_center(s_grade);lv_obj_set_style_text_color(s_grade,lv_color_hex(0x904B3D),0);lv_obj_set_style_text_line_space(s_grade,14,0);
    s_body=label(s_group[2],"");lv_obj_set_size(s_body,156,176);lv_obj_set_pos(s_body,138,25);lv_obj_set_style_text_line_space(s_body,10,0);
    lv_obj_t *hint=label(s_panel,muse_i18n_text(MUSE_KEY_s051));lv_obj_align(hint,LV_ALIGN_BOTTOM_MID,0,-15);lv_obj_set_style_text_color(hint,lv_color_hex(0xB9B8AD),0);
    lv_obj_add_event_cb(canvas,on_canvas,LV_EVENT_ALL,NULL);
}
void muse_pet_ui_tick(bool visible,muse_mode_t mode,float now) {
    if(!s_hud)return;
    s_now=now;
    s_touch_allowed=visible&&mode!=MUSE_MODE_OFF&&!muse_state_asleep();s_allowed=s_touch_allowed&&mode==MUSE_MODE_IDLE;
    lv_obj_t *keys[]={s_hud,s_coin,s_feed,s_fortune};for(unsigned i=0;i<4;i++)lv_obj_set_flag(keys[i],LV_OBJ_FLAG_HIDDEN,!s_allowed||!muse_launcher_entries_visible());
    if(!s_allowed){close_modal(NULL);s_kind=PET_ANIM_NONE;s_anim=(muse_pet_anim_frame_t){0};lv_obj_add_flag(s_food,LV_OBJ_FLAG_HIDDEN);lv_obj_add_flag(s_toast,LV_OBJ_FLAG_HIDDEN);s_coin_at=-100;muse_pixel_set_interaction(0);}
    muse_pet_feedback_t f;
    while(muse_pet_feedback(&f)) {
        if(f.action==PET_SFX_OFF||f.action==PET_SFX_ON)continue;
        s_pending=false;if(!s_allowed)continue;
        if(f.result==PET_STORAGE){toast(muse_i18n_text(MUSE_KEY_s037));continue;}
        if(f.action==PET_TOUCH) {
            if(f.result==PET_COIN){toast(muse_i18n_text(MUSE_KEY_s041));s_coin_at=now;s_coin_landed=false;muse_voice_request_pet_sfx(PET_SFX_REWARD);}
            else if(f.result==PET_CAP){toast(time(NULL)<MUSE_PET_MIN_TIME?muse_i18n_text(MUSE_KEY_s039):muse_i18n_text(MUSE_KEY_s033));muse_voice_request_pet_sfx(PET_SFX_POP);}
            else if(f.result==PET_TIME)toast(muse_i18n_text(MUSE_KEY_s042));
        } else if(muse_pet_food(f.action)) {
            s_anim_at=now;s_last_chew=-1;
            s_kind=f.result==PET_FULL?PET_ANIM_FULL:f.result==PET_POOR?PET_ANIM_POOR:f.action==PET_COFFEE?PET_ANIM_COFFEE:f.action==PET_CAKE?PET_ANIM_CAKE:f.action==PET_RICE?PET_ANIM_RICE:f.action==PET_HOTPOT?PET_ANIM_HOTPOT:f.action==PET_CHICKEN?PET_ANIM_CHICKEN:PET_ANIM_TEA;
            if(f.result==PET_FULL||f.result==PET_POOR){toast(f.result==PET_FULL?muse_i18n_text(MUSE_KEY_s038):muse_i18n_text(MUSE_KEY_s049));muse_voice_request_pet_sfx(PET_SFX_REFUSE);}
        } else if(f.action==PET_FORTUNE) {
            const char *g=muse_pet_fortune_grade(f.fortune);lv_label_set_text(s_grade,g);
            const char *body=muse_pet_fortune(f.fortune),*sep=strstr(body,muse_i18n_language()==MUSE_LANG_EN?": ":"：");lv_label_set_text(s_body,sep?sep+(muse_i18n_language()==MUSE_LANG_EN?2:3):body);show_page(2,muse_i18n_text(MUSE_KEY_s036));
            if(f.result==PET_FORTUNE_NEW){s_flip_until=now+.45f;muse_voice_request_pet_sfx(PET_SFX_CARD);}
        }
    }
    muse_pet_model_t pet;
    if(muse_pet_snapshot(&pet)) {
        if(pet.coins!=s_last_coins){lv_label_set_text_fmt(s_hud,"%lu",(unsigned long)pet.coins);s_last_coins=pet.coins;}
        if(s_open&&s_page==0)update_card(&pet);
    }
    if(s_open&&s_flip_until>0){float x=1-(s_flip_until-now)/.45f;if(x<0)x=0;if(x>1)x=1;lv_obj_set_width(s_panel,28+(int)(306*x));if(x>=1)s_flip_until=0;}
    if(now>s_toast_until)lv_obj_add_flag(s_toast,LV_OBJ_FLAG_HIDDEN);
    muse_hint_panel_sync(&s_hint,s_allowed);
    float u=(now-s_coin_at)/.9f;
    lv_obj_set_flag(s_flying,LV_OBJ_FLAG_HIDDEN,!s_allowed||u<0||u>1);
    if(s_allowed&&u>=0&&u<=1){lv_obj_align(s_flying,LV_ALIGN_CENTER,(int)(-142*(1-u)),(int)(-116+76*u-46*sinf(u*3.14159265f)));}
    if(s_allowed&&s_coin_at>=0&&u>=1&&!s_coin_landed){s_coin_landed=true;muse_state_make_happy();}
    s_anim=muse_pet_anim_sample(s_kind,now-s_anim_at);
    if(s_kind!=PET_ANIM_NONE&&!s_anim.active){s_kind=PET_ANIM_NONE;lv_obj_add_flag(s_food,LV_OBJ_FLAG_HIDDEN);}
    if(s_anim.active){
        muse_state_poke();lv_obj_set_flag(s_food,LV_OBJ_FLAG_HIDDEN,!s_anim.food);lv_obj_align(s_food,LV_ALIGN_CENTER,52,146+(int)s_anim.food_y);lv_obj_set_style_opa(s_food,(lv_opa_t)(s_anim.food_opa*255),0);
        if(s_anim.happy>0&&now-s_anim_at<4.1f)muse_state_make_happy();
        int64_t chew=(int64_t)((now-s_anim_at-1.4f)/.44f);
        if(now-s_anim_at>=1.4f&&now-s_anim_at<4&&chew!=s_last_chew){s_last_chew=chew;muse_voice_request_pet_sfx(PET_SFX_CHEW);}
    }
}
bool muse_pet_ui_hint_active(void){return s_hint.frame&&!lv_obj_has_flag(s_hint.frame,LV_OBJ_FLAG_HIDDEN);}
void muse_pet_ui_pose(muse_pose_t *pose,float now) {
    muse_pixel_set_interaction(0);if(pose->mode!=MUSE_MODE_IDLE||!s_allowed)return;
    if(s_anim.active) {
        if(s_anim.refuse){pose->mode=MUSE_MODE_ERROR;pose->happy=0;}
        else if(now-s_anim_at>=1.4f&&now-s_anim_at<4){muse_pixel_set_interaction(1);pose->level=s_anim.mouth;pose->happy=0;}
        else if(s_anim.happy>0)pose->happy=1;
    }else {muse_pet_model_t s;if(muse_pet_snapshot(&s)&&s.hunger<30000&&pose->happy<.05f)muse_pixel_set_interaction(2);}
}
void muse_pet_ui_place(int base_y) {
    int x=s_anim.active?(int)lroundf(s_anim.x):0;if(x||s_offset){lv_obj_align(s_canvas,LV_ALIGN_CENTER,x,base_y);s_offset=x;}
}
#endif
