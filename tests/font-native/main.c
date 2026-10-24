/* Exercise actual compiled LVGL glyph lookup, UI fallback and caption labels. */
#include "lvgl.h"
#include "muse_ui_fonts.h"
#include "muse_i18n.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
LV_FONT_DECLARE(muse_font_noto_tc_16)
static void flush(lv_display_t *display,const lv_area_t *area,uint8_t *pixels){(void)area;(void)pixels;lv_display_flush_ready(display);}
static uint32_t next(const unsigned char **p){uint32_t c=*(*p)++;if(c<128)return c;unsigned n=(c&0xe0)==0xc0?1:(c&0xf0)==0xe0?2:3;c&=n==1?31:n==2?15:7;while(n--){assert((**p&0xc0)==0x80);c=(c<<6)|(*(*p)++&63);}return c;}
static unsigned check(const char *text,const lv_font_t *font){unsigned n=0;const unsigned char *p=(const unsigned char*)text;while(*p){uint32_t c=next(&p);if(c=='\n'||c=='\r')continue;lv_font_glyph_dsc_t d;assert(lv_font_get_glyph_dsc(font,&d,c,0));assert(!d.is_placeholder);n++;}return n;}
static char *read_all(const char *path){FILE *f=fopen(path,"rb");assert(f);fseek(f,0,SEEK_END);long n=ftell(f);rewind(f);char *s=calloc(n+1,1);assert(s&&fread(s,1,n,f)==(size_t)n);fclose(f);return s;}
int main(int argc,char **argv){assert(argc==4);lv_init();lv_display_t *d=lv_display_create(720,160);static uint8_t pixels[720*160*2];lv_display_set_buffers(d,pixels,NULL,sizeof pixels,LV_DISPLAY_RENDER_MODE_FULL);lv_display_set_flush_cb(d,flush);
 /* Mixed-language Latin must stay proportional; CJK retains its full cell. */
 assert(lv_font_get_glyph_width(&muse_font_noto_tc_16,'i',0)<lv_font_get_glyph_width(&muse_font_noto_tc_16,'M',0));
 assert(lv_font_get_glyph_width(&muse_font_noto_tc_16,'M',0)<lv_font_get_glyph_width(&muse_font_noto_tc_16,0x6F22,0));
 assert(lv_font_get_glyph_width(&muse_font_noto_tc_16,'1',0)<16);
 /* Check every translated fixed string against the bundled font. */
#define MUSE_STRING(id,en,cn,tw) check(en,&muse_font_noto_tc_16);check(cn,&muse_font_noto_tc_16);check(tw,&muse_font_noto_tc_16);
#include "muse_i18n_catalog.def"
#undef MUSE_STRING
 char *charset=read_all(argv[1]);unsigned total=check(charset,&muse_font_noto_tc_16);free(charset);
 char *sample=read_all(argv[2]);const lv_font_t *font=muse_ui_font(!strcmp(argv[3],"en")?&lv_font_unscii_8:&muse_font_noto_tc_16);unsigned count=check(sample,font);
 lv_obj_t *panel=lv_obj_create(lv_screen_active());lv_obj_set_size(panel,720,160);lv_obj_set_style_bg_color(panel,lv_color_black(),0);lv_obj_set_style_bg_opa(panel,LV_OPA_COVER,0);lv_obj_set_style_border_width(panel,0,0);lv_obj_set_style_pad_all(panel,12,0);lv_obj_remove_flag(panel,LV_OBJ_FLAG_SCROLLABLE);
 lv_obj_t *label=lv_label_create(panel);lv_obj_set_width(label,696);lv_obj_set_style_text_font(label,font,0);lv_obj_set_style_text_color(label,lv_color_white(),0);lv_label_set_long_mode(label,LV_LABEL_LONG_MODE_WRAP);lv_label_set_text(label,sample);lv_obj_update_layout(panel);assert(!strcmp(sample,lv_label_get_text(label)));assert(lv_obj_get_height(label)<=136);
 lv_draw_buf_t *snap=lv_snapshot_take(panel,LV_COLOR_FORMAT_RGB888);assert(snap);FILE *f=fopen("caption.ppm","wb");assert(f);fprintf(f,"P6\n%u %u\n255\n",snap->header.w,snap->header.h);for(unsigned y=0;y<snap->header.h;y++)for(unsigned x=0;x<snap->header.w;x++){const uint8_t *p=snap->data+y*snap->header.stride+x*3;fputc(p[2],f);fputc(p[1],f);fputc(p[0],f);}fclose(f);lv_draw_buf_destroy(snap);printf("coverage=%u sample=%u language=%s missing=0 unchanged_text=1\n",total,count,argv[3]);free(sample);lv_deinit();return 0;}
