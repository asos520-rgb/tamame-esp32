/* Public-edition staging: local UI additions and/or privacy review; see source manifest. */
/* Shared, image-free pixel chrome for the 175C system controls. */
#pragma once
#include "lvgl.h"
#include <stdint.h>
typedef enum { MUSE_ICON_MIC, MUSE_ICON_SPEAKER, MUSE_ICON_MUTE, MUSE_ICON_POWER } muse_control_icon_t;
static inline void muse_pixel_control_draw(lv_layer_t *layer, const lv_area_t *box,
                                            muse_control_icon_t kind, uint32_t color, bool lit)
{
    lv_draw_rect_dsc_t d;lv_draw_rect_dsc_init(&d);
    d.bg_opa=LV_OPA_COVER;d.radius=LV_RADIUS_CIRCLE;
    d.border_width=1;d.border_color=lv_color_hex(lit ? color : 0x596176);
    d.bg_color=lv_color_hex(lit ? 0x2D3343 : 0x222734);
    lv_draw_rect(layer,&d,box);
    static const char glyphs[3][16][17]={
      {"                ","      ssss      ","     sWWWgs     ","     sWgggs     ","     sgsggs     ","     sggsgs     ","     sgsggs     ","   s sggggs s   ","   g  ssss g    ","   sg     gs    ","    sgggggs     ","      ss        ","      gg        ","     sggs       ","   ssYYYYss     ","                "},
      {"                ","                ","       Y        ","      YW    b   ","     YWW   b b  ","  OOOYWg  b   b ","  bbbYWg  b   b ","  bWbYWg  b   b ","  bbbYWg  b   b ","  OOOYWg  b   b ","     YWW   b b  ","      YW    b   ","       Y        ","                ","                ","                "},
      {"                ","                ","       Y        ","      YW        ","     YWW        ","  OOOYWg  R   R ","  bbbYWg   R R  ","  bWbYWg    R   ","  bbbYWg   R R  ","  OOOYWg  R   R ","     YWW        ","      YW        ","       Y        ","                ","                ","                "}
    };
    if(kind>MUSE_ICON_MUTE)return;
    int width=lv_area_get_width(box),height=lv_area_get_height(box);
    int scale=(width<height?width:height)/20;if(scale<1)scale=1;
    int left=box->x1+(width-16*scale)/2,top=box->y1+(height-16*scale)/2;
    d.radius=0;d.border_width=0;
    for(int y=0;y<16;y++)for(int x=0;x<16;x++) {
        uint32_t rgb;
        switch(glyphs[kind][y][x]) {
        case 's':rgb=0x596A80;break;case 'g':rgb=0xA9B7C8;break;
        case 'W':rgb=0xFFF2CE;break;case 'b':rgb=0x7DADCB;break;
        case 'Y':rgb=0xF6CB4B;break;case 'O':rgb=0x644331;break;
        case 'R':rgb=0xBD6655;break;default:continue;
        }
        if(kind==MUSE_ICON_MIC && lit && rgb==0xA9B7C8)rgb=color;
        d.bg_color=lv_color_hex(rgb);
        lv_area_t a={left+x*scale,top+y*scale,left+(x+1)*scale-1,top+(y+1)*scale-1};
        lv_draw_rect(layer,&d,&a);
    }
}

typedef struct {lv_obj_t *obj;muse_control_icon_t kind;uint32_t color;bool lit;} muse_pixel_control_t;
static void muse_pixel_control_event(lv_event_t *e)
{
    muse_pixel_control_t *c=lv_event_get_user_data(e);lv_area_t box;
    lv_event_stop_processing(e);lv_obj_get_coords(c->obj,&box);
    muse_pixel_control_draw(lv_event_get_layer(e),&box,c->kind,c->color,c->lit);
}
static inline void muse_pixel_control_attach(muse_pixel_control_t *c,lv_obj_t *obj,
                                              muse_control_icon_t kind,uint32_t color)
{
    *c=(muse_pixel_control_t){.obj=obj,.kind=kind,.color=color};
    lv_obj_add_event_cb(obj,muse_pixel_control_event,LV_EVENT_DRAW_MAIN|LV_EVENT_PREPROCESS,c);
}
