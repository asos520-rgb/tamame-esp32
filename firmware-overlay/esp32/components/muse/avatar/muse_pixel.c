/* Public-edition staging: local UI additions and/or privacy review; see source manifest. */
/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/* Modified for public template preparation: configurable skin and fixed anchors;
 * unbranded clothing; five generic scenes. Not yet integrated into firmware. */
#include "muse_pixel.h"
#include <math.h>
#include <string.h>
#include "portrait_assets.h"
#include "avatar_profile.h"
#if PET_AVATAR_TEMPLATE_VERSION != 1
#error Unsupported avatar template
#endif
#define PET_SCENE_COUNT 5
#ifdef ESP_PLATFORM
#include "sdkconfig.h"
#if CONFIG_SPIRAM
#include "esp_attr.h"
#define STYLE_BIG EXT_RAM_BSS_ATTR
#endif
#endif
#ifndef STYLE_BIG
#define STYLE_BIG
#endif
#define W MUSE_PX_W
#define H MUSE_PX_H
#define TAU 6.28318530718f
enum { A_BOOT,A_IDLE,A_LISTEN,A_THINK,A_SPEAK,A_HAPPY,A_ERROR,A_OFF };
#include "style_scene_assets.h"
#include "style_silhouette.h"
static STYLE_BIG uint8_t coverage[64*64];
static unsigned style_clothing,style_accessory,style_scene;
static bool style_glasses;
static int scene_x,scene_y;
void muse_pixel_set_style(unsigned clothing,unsigned accessory,bool sunglasses){style_clothing=clothing<3?clothing:0;style_accessory=accessory<3?accessory:0;style_glasses=sunglasses;}
void muse_pixel_set_scene(unsigned scene,int x,int y){style_scene=scene<=PET_SCENE_COUNT?scene:0;scene_x=x;scene_y=y;}
uint16_t muse_pixel_scene_pixel(unsigned scene,int x,int y){
 if(!scene||scene>PET_SCENE_COUNT||x<1||x>=465||y<1||y>=465)return 0;
 unsigned at=(unsigned)((y-1)/4)*116+(unsigned)((x-1)/4);
 return style_scene_palette[scene-1][style_scene_pixels[scene-1][at]];
}
static uint16_t fb[W*H];
static int size_px=W;
static float last_t=-1, mouth_level;
static bool mouth_open;
static int interaction;
void muse_pixel_set_interaction(int state){interaction=state>=0&&state<=2?state:0;}
static float clamp(float x,float lo,float hi) {return x<lo?lo:x>hi?hi:x;}
static uint16_t color(unsigned rgb,float gain) {
 unsigned r=(unsigned)(((rgb>>16)&255)*gain),g=(unsigned)(((rgb>>8)&255)*gain),b=(unsigned)((rgb&255)*gain);
 return (uint16_t)(((r>>3)<<11)|((g>>2)<<5)|(b>>3));
}
static uint16_t dim(uint16_t c,float gain) {
 unsigned r=(unsigned)(((c>>11)&31)*gain),g=(unsigned)(((c>>5)&63)*gain),b=(unsigned)((c&31)*gain);
 return (uint16_t)((r<<11)|(g<<5)|b);
}
static void point(int x,int y,uint16_t c) {if(x>=0&&x<W&&y>=0&&y<H){fb[y*W+x]=c;coverage[y*W+x]=1;}}
static void background_point(int x,int y,uint16_t c) {if(x>=0&&x<W&&y>=0&&y<H&&!coverage[y*W+x])point(x,y,c);}
static void sparkle(int x,int y,int r,float gain) {
 uint16_t bright=color(0xD4AAFF,gain),edge=color(0x9F62DC,gain);
 background_point(x,y,bright);
 for(int i=1;i<=r;i++) {
  background_point(x+i,y,edge);background_point(x-i,y,edge);
  background_point(x,y+i,edge);background_point(x,y-i,edge);
 }
}
static void heart(int x,int y,float gain) {
 const unsigned bits[]={0x0a,0x1f,0x1f,0x0e,0x04};
 uint16_t c=color(0xF493BC,gain);
 for(int row=0;row<5;row++)for(int col=0;col<5;col++)
  if(bits[row]&(1u<<col))background_point(x+col-2,y+row,c);
}
static void sprite(int art,int dx,int dy) {
 for(int y=0;y<H;y++)for(int x=0;x<W;x++) {
  uint16_t c=portrait_palette[portrait_images[art][y*W+x]];
  if ((style_silhouette[art][(y*W+x)/8] >> ((y*W+x)%8)) & 1) point(x + dx, y + dy, c);
 }
}
static void patch(int art,int x0,int y0,int x1,int y1,int dx,int dy) {
 for(int y=y0;y<=y1;y++)for(int x=x0;x<=x1;x++)
  if ((style_silhouette[art][(y*W+x)/8] >> ((y*W+x)%8)) & 1)
   point(x+dx,y+dy,portrait_palette[portrait_images[art][y*W+x]]);
}
/* Closed eyelids come from the off-state artwork, not redrawn geometry. */
static void eyelids(float amount,int dx,int dy) {
 int last=PET_AVATAR_LEFT_EYE_Y0+(int)(clamp(amount,0,1)*(PET_AVATAR_LEFT_EYE_Y1-PET_AVATAR_LEFT_EYE_Y0));
 patch(A_OFF,PET_AVATAR_LEFT_EYE_X0,PET_AVATAR_LEFT_EYE_Y0,PET_AVATAR_LEFT_EYE_X1,last,dx,dy);patch(A_OFF,PET_AVATAR_RIGHT_EYE_X0,PET_AVATAR_RIGHT_EYE_Y0,PET_AVATAR_RIGHT_EYE_X1,last,dx,dy);
}
uint32_t muse_pixel_accent(muse_mode_t mode) {
 static const uint32_t colors[]={0xBFA1FF,0xB58AF5,0x8CCBFF,0xC898F5,0x9DE8C8,0xD5A183,0x74698B};
 return colors[(unsigned)mode<MUSE_MODE_COUNT?(unsigned)mode:MUSE_MODE_IDLE];
}
void muse_pixel_render(const muse_pose_t *p) {
 if(!p)return;
 memset(fb, 0, sizeof(fb));
    memset(coverage,0,sizeof coverage);
 float t=isfinite(p->t)?p->t:0,mt=isfinite(p->mode_t)?fmaxf(0,p->mode_t):0;
 float level=isfinite(p->level)?clamp(p->level,0,1):0;
 float happy=isfinite(p->happy)?clamp(p->happy,0,1):0;
 muse_mode_t mode=(unsigned)p->mode<MUSE_MODE_COUNT?p->mode:MUSE_MODE_IDLE;
 float dt=last_t<0||t<last_t?.04f:clamp(t-last_t,0,.15f);last_t=t;
 mouth_level+=(level-mouth_level)*clamp(dt*9,0,1);
 if(mouth_level>.20f)mouth_open=true;else if(mouth_level<.11f)mouth_open=false;
 int art=A_IDLE,dx=0,dy=0;
 bool joy=happy>.05f&&mode!=MUSE_MODE_ERROR&&mode!=MUSE_MODE_OFF&&mode!=MUSE_MODE_BOOT;
 if(joy){art=A_HAPPY;dy=-(int)lroundf(fabsf(sinf(t*5))*2*happy);}
 else switch(mode) {
 case MUSE_MODE_BOOT:art=mt<.7f?A_BOOT:A_IDLE;break;
 case MUSE_MODE_IDLE:dy=(int)lroundf(sinf(t*TAU/4)*.65f);break;
 case MUSE_MODE_LISTENING:art=A_LISTEN;dy=1;break;
 case MUSE_MODE_THINKING:art=A_THINK;dx=(int)lroundf(sinf((t-.15f)*TAU/4)*.6f);break;
 case MUSE_MODE_SPEAKING:art=A_SPEAK;dy=(int)lroundf(sinf(t*TAU/3)*.55f);break;
 case MUSE_MODE_ERROR:art=A_ERROR;break;
 case MUSE_MODE_OFF:art=A_OFF;dy=mt>.5f?1:0;break;
 default:break;
 }
 bool eating=mode==MUSE_MODE_IDLE&&interaction==1;
 bool hungry=mode==MUSE_MODE_IDLE&&interaction==2&&!joy;
 if(eating){art=A_SPEAK;dy=(int)lroundf(sinf(t*TAU/0.88f));}
 sprite(art,dx,dy);
    /* Body-only overlays: head pixels remain untouched. */
    if(style_clothing){
      for(int y=PET_AVATAR_CLOTHING_Y0;y<=PET_AVATAR_CLOTHING_Y1;y++)for(int x=PET_AVATAR_CLOTHING_X0;x<=PET_AVATAR_CLOTHING_X1;x++){
        int px=x+dx,py=y+dy;if(px<0||px>=64||py<0||py>=64||!coverage[py*64+px])continue;
        bool apron=style_clothing==2;
        uint32_t rgb=apron?((x==24||x==39)?0x9C7653:0xC8A47D):((x<23||x>40)?0x132A43:0x203E60);
        if(apron&&(x<24||x>39)&&y<51)rgb=0xE6DDD0;
        point(px,py,color(rgb,1));
      }
      if(style_clothing==2){for(int x=27;x<37;x++)point(x+dx,54+dy,color(0xAB845E,1));}
    }
    if(style_accessory==1){
      for(int x=24;x<40;x++)for(int y=46;y<48;y++)point(x+dx,y+dy,color(0xA85C4E,1));
      for(int y=48;y<54;y++)for(int x=25;x<28;x++)point(x+dx,y+dy,color(y%2?0xC77B60:0xA85C4E,1));
    }else if(style_accessory==2){
      for(int y=47;y<56;y++)point(24+(y-47)*2+dx,y+dy,color(0x996F4C,1));
      for(int y=53;y<58;y++)for(int x=35;x<43;x++)point(x+dx,y+dy,color(y==53?0xBA9265:0x805D40,1));
    }


 if(eating){
  /* Independent food reaction: squinting eyes, puffed cheeks, raised hands. */
  patch(A_HAPPY,PET_AVATAR_LEFT_EYE_X0,PET_AVATAR_LEFT_EYE_Y0,PET_AVATAR_LEFT_EYE_X1,(PET_AVATAR_LEFT_EYE_Y1-1),dx,dy);patch(A_HAPPY,PET_AVATAR_RIGHT_EYE_X0,PET_AVATAR_RIGHT_EYE_Y0,PET_AVATAR_RIGHT_EYE_X1,(PET_AVATAR_RIGHT_EYE_Y1-1),dx,dy);
  for(int y=39;y<42;y++)for(int x=20;x<24;x++){point(x+dx,y+dy,color(PET_AVATAR_CHEEK,.9f));point(x+19+dx,y+dy,color(PET_AVATAR_CHEEK,.9f));}
  for(int y=47;y<50;y++)for(int x=25;x<29;x++){point(x+dx,y+dy,color(PET_AVATAR_SKIN,1));point(x+10+dx,y+dy,color(PET_AVATAR_SKIN,1));}
  for(int y=44;y<48;y++)for(int x=29;x<35;x++)point(x+dx,y+dy,color(y==44?0xF6CB4B:0xCA945F,1));
  if(!mouth_open)patch(A_IDLE,PET_AVATAR_MOUTH_X0,PET_AVATAR_MOUTH_Y0,PET_AVATAR_MOUTH_X1,(PET_AVATAR_MOUTH_Y1-1),dx,dy);
 }
 if(hungry){
  patch(A_ERROR,PET_AVATAR_LEFT_EYE_X0,PET_AVATAR_LEFT_EYE_Y0,PET_AVATAR_LEFT_EYE_X1,(PET_AVATAR_LEFT_EYE_Y1-1),dx,dy);patch(A_ERROR,PET_AVATAR_RIGHT_EYE_X0,PET_AVATAR_RIGHT_EYE_Y0,PET_AVATAR_RIGHT_EYE_X1,(PET_AVATAR_RIGHT_EYE_Y1-1),dx,dy);
  patch(A_IDLE,PET_AVATAR_MOUTH_X0,PET_AVATAR_MOUTH_Y0,PET_AVATAR_MOUTH_X1,PET_AVATAR_MOUTH_Y1,dx,dy);
  point(29+dx,41+dy,color(PET_AVATAR_MOUTH,1));point(30+dx,40+dy,color(PET_AVATAR_MOUTH,1));point(31+dx,40+dy,color(PET_AVATAR_MOUTH,1));point(32+dx,41+dy,color(PET_AVATAR_MOUTH,1));
  /* Empty rice bowl and a soft sweat drop, independent of the error state. */
  for(int y=47;y<51;y++)for(int x=53+(y-47)/2;x<61-(y-47)/2;x++)point(x,y,color(0xDDAF63,.85f));
  point(47,29,color(0x7BBBC9,1));point(46,30,color(0x7BBBC9,1));point(47,30,color(0x7BBBC9,1));
 }
 if(!joy) {
  float blink=fmodf(fmaxf(0,t),mode==MUSE_MODE_ERROR?6.0f:5.0f);
  if(mode!=MUSE_MODE_BOOT&&mode!=MUSE_MODE_OFF&&blink>.9f&&blink<1.22f) {
   float u=blink-.9f;
   eyelids(u<.08f?u/.08f:u<.24f?1:(.32f-u)/.08f,dx,dy);
  }
  if(mode==MUSE_MODE_SPEAKING&&!mouth_open)patch(A_IDLE,PET_AVATAR_MOUTH_X0,PET_AVATAR_MOUTH_Y0,PET_AVATAR_MOUTH_X1,(PET_AVATAR_MOUTH_Y1-1),dx,dy);
  if(mode==MUSE_MODE_THINKING&&sinf(t*TAU/4)>.65f) {
   patch(A_IDLE,PET_AVATAR_LEFT_EYE_X0,PET_AVATAR_LEFT_EYE_Y0,PET_AVATAR_LEFT_EYE_X1,PET_AVATAR_LEFT_EYE_Y1,dx,dy);patch(A_IDLE,PET_AVATAR_RIGHT_EYE_X0,PET_AVATAR_RIGHT_EYE_Y0,PET_AVATAR_RIGHT_EYE_X1,PET_AVATAR_RIGHT_EYE_Y1,dx,dy);
  }
 }
 if(mode==MUSE_MODE_IDLE&&!joy) {
  for(int i=0;i<3;i++) {
   float a=t*TAU/7+i*TAU/3;
   sparkle((int)lroundf(32+27*cosf(a)),(int)lroundf(32+25*sinf(a)),i==1?1:2,
           .55f+.4f*(.5f+.5f*sinf(t*2+i*2)));
  }
 } else if(mode==MUSE_MODE_THINKING&&!joy) {
  int active=(int)(fmaxf(0,t)*1.6f)%3;
  for(int i=0;i<3;i++)point(53+i*3,12,color(0xB98AEA,i==active?1:.35f));
 } else if(mode==MUSE_MODE_LISTENING&&!joy) {
  float gain=.4f+.6f*level;
  for(int side=-1;side<=1;side+=2) {
   int x=32+side*25;
   background_point(x,31,color(0xBC9AEF,gain));
   background_point(x,33,color(0xBC9AEF,gain));
   background_point(x+side,32,color(0xBC9AEF,gain));
  }
 } else if(mode==MUSE_MODE_ERROR) {
  sparkle(15,47,1,.25f+.25f*(.5f+.5f*sinf(t)));
 }
 if(joy)for(int i=0;i<2;i++) {
  float u=fmodf(fmaxf(0,t)*.65f+i*.5f,1);
  heart(i?54:9,18-(int)(u*12),happy*(1-u));
 }
 if(mode==MUSE_MODE_BOOT) {
  for(int i=0;i<2;i++)sparkle(i?52:12,45-i*4,1,.5f);
 }
    if(style_glasses){
      for(int y=32;y<36;y++)for(int x=19;x<44;x++)if(x<29||x>33||y==33)point(x+dx,y+dy,color(y==32?0x474957:0x111724,1));
      point(21+dx,33+dy,color(0x8E9BAB,1));point(36+dx,33+dy,color(0x8E9BAB,1));
    }
 float fade=mode==MUSE_MODE_BOOT?clamp(mt/1.2f,0,1):mode==MUSE_MODE_OFF?1-clamp((mt-.6f)/1.2f,0,1):1;
 if(fade<1)for(int i=0;i<W*H;i++)fb[i]=dim(fb[i],fade);
}
void muse_pixel_set_size(int px) {size_px=px>0?px:1;}
void muse_pixel_scale(uint16_t *dst,int stride,int x0,int x1,int y0,int y1) {
 if(!dst||stride<=0||x0<0||y0<0||x1<x0||y1<y0||x1>=size_px||y1>=size_px||stride<x1-x0+1)return;
 for(int y=y0;y<=y1;y++,dst+=stride) {
  int sy=(int)((int64_t)y*H/size_px);
  for(int x=x0;x<=x1;x++){
   int at=sy*W+(int)((int64_t)x*W/size_px);
   dst[x-x0]=coverage[at]?fb[at]:muse_pixel_scene_pixel(style_scene,scene_x+x,scene_y+y);
  }
 }
}

/* UI-only preview transaction: animation timing, mouth hysteresis, scene origin,
 * framebuffer and coverage all restored before LVGL draws the main avatar. */
void muse_pixel_style_preview(uint16_t *pixels,unsigned background,unsigned clothing,unsigned accessory,bool glasses){
 if(!pixels)return;
 static STYLE_BIG uint16_t saved_fb[4096];static STYLE_BIG uint8_t saved_coverage[4096];
 memcpy(saved_fb,fb,sizeof fb);memcpy(saved_coverage,coverage,sizeof coverage);
 unsigned old_cloth=style_clothing,old_acc=style_accessory,old_scene=style_scene;
 bool old_glasses=style_glasses,old_mouth=mouth_open;
 int old_size=size_px,old_x=scene_x,old_y=scene_y,old_interaction=interaction;
 float old_t=last_t,old_level=mouth_level;
 muse_pixel_set_style(clothing,accessory,glasses);interaction=0;
 muse_pose_t pose={.mode=MUSE_MODE_IDLE,.t=2,.mode_t=2};muse_pixel_render(&pose);
 for(int y=0;y<64;y++)for(int x=0;x<64;x++)pixels[y*64+x]=coverage[y*64+x]?fb[y*64+x]:muse_pixel_scene_pixel(background,x*466/64,y*466/64);
 memcpy(fb,saved_fb,sizeof fb);memcpy(coverage,saved_coverage,sizeof coverage);
 style_clothing=old_cloth;style_accessory=old_acc;style_scene=old_scene;style_glasses=old_glasses;mouth_open=old_mouth;size_px=old_size;scene_x=old_x;scene_y=old_y;interaction=old_interaction;last_t=old_t;mouth_level=old_level;
}
