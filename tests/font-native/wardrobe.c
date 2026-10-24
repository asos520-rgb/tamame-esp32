/* Real production LVGL wardrobe layout; renderer/storage stand-ins only. */
#include "muse_style_ui.c"
#include <assert.h>
#include <string.h>
static muse_style_t saved={.version=2,.accessory=1};
static int feedback;
void muse_state_poke(void){}
bool muse_state_asleep(void){return false;}
void muse_launcher_add(unsigned id,lv_obj_t *button){(void)id;(void)button;}
void muse_launcher_modal(lv_obj_t *modal){(void)modal;}
bool muse_launcher_entries_visible(void){return true;}
bool muse_style_snapshot(muse_style_t *out){*out=saved;return true;}
bool muse_style_save(const muse_style_t *style){assert(muse_style_valid(style));saved=*style;feedback=1;return true;}
int muse_style_feedback(void){int result=feedback;feedback=0;return result;}
void muse_pixel_set_size(int px){(void)px;}
void muse_pixel_set_style(unsigned c,unsigned a,bool g){(void)c;(void)a;(void)g;}
void muse_pixel_set_scene(unsigned s,int x,int y){(void)s;(void)x;(void)y;}
uint16_t muse_pixel_scene_pixel(unsigned s,int x,int y){(void)s;(void)x;(void)y;return 0;}
void muse_pixel_style_preview(uint16_t *pixels,unsigned b,unsigned c,unsigned a,bool g){(void)b;(void)c;(void)a;(void)g;memset(pixels,0,64*64*2);}
static void flush(lv_display_t *d,const lv_area_t *a,uint8_t *p){(void)a;(void)p;lv_display_flush_ready(d);}
static void click(lv_obj_t *button){lv_obj_send_event(button,LV_EVENT_CLICKED,NULL);lv_obj_update_layout(lv_layer_top());}
static void tick(void){muse_style_ui_tick(true,MUSE_MODE_IDLE);lv_obj_update_layout(lv_layer_top());}
static void check_button(lv_obj_t *button){
 lv_obj_t *label=lv_obj_get_child(button,0);lv_point_t size;
 lv_text_get_size(&size,lv_label_get_text(label),lv_obj_get_style_text_font(label,0),0,0,lv_obj_get_width(label),LV_TEXT_FLAG_NONE);
 if(size.y>lv_obj_get_height(button)-4)fprintf(stderr,"text=%s text_size=%ld,%ld label_w=%ld button=%ld,%ld\n",lv_label_get_text(label),(long)size.x,(long)size.y,(long)lv_obj_get_width(label),(long)lv_obj_get_width(button),(long)lv_obj_get_height(button));
 assert(size.y<=lv_obj_get_height(button)-4);
 lv_area_t b,l;lv_obj_get_coords(button,&b);lv_obj_get_coords(label,&l);
 assert(l.x1>=b.x1&&l.x2<=b.x2&&l.y1>=b.y1&&l.y2<=b.y2);
}
static void snapshot(unsigned category){
 lv_draw_buf_t *image=lv_snapshot_take(ws_modal,LV_COLOR_FORMAT_RGB888);assert(image);
 char path[64];snprintf(path,sizeof path,"wardrobe-%u.ppm",category);FILE *f=fopen(path,"wb");assert(f);
 fprintf(f,"P6\n%u %u\n255\n",image->header.w,image->header.h);
 for(unsigned y=0;y<image->header.h;y++)for(unsigned x=0;x<image->header.w;x++){const uint8_t *p=image->data+y*image->header.stride+x*3;fputc(p[2],f);fputc(p[1],f);fputc(p[0],f);}fclose(f);lv_draw_buf_destroy(image);
}
int main(int argc,char **argv){
 assert(argc==2);assert(muse_i18n_set_language((unsigned)atoi(argv[1])));lv_init();
 lv_display_t *display=lv_display_create(466,466);static uint8_t buffer[466*466*2];lv_display_set_buffers(display,buffer,NULL,sizeof buffer,LV_DISPLAY_RENDER_MODE_FULL);lv_display_set_flush_cb(display,flush);
 lv_obj_t *face=lv_obj_create(lv_screen_active());lv_obj_remove_style_all(face);lv_obj_set_size(face,466,466);lv_obj_t *avatar=lv_obj_create(face);lv_obj_set_size(avatar,64,64);
 muse_style_ui_build(face,avatar);tick();muse_style_ui_request(1,0);tick();assert(ws_opened);
 for(unsigned category=0;category<3;category++){
  click(ws_tabs[category]);assert(ws_category==category);
  for(unsigned i=0;i<3;i++)check_button(ws_tabs[i]);
  for(unsigned i=0;i<ws_counts[category];i++)check_button(ws_choices[i]);
  assert(lv_obj_get_height(ws_note)<=21);snapshot(category);
 }
 /* Accessory single-selection and independent glasses toggle must coexist. */
 click(ws_choices[3]);assert(ws_draft.accessory==1&&ws_draft.sunglasses==1);
 click(ws_choices[2]);assert(ws_draft.accessory==2&&ws_draft.sunglasses==1);
 click(ws_choices[0]);assert(ws_draft.accessory==0&&ws_draft.sunglasses==1);
 click(ws_choices[1]);click(ws_choices[3]);assert(ws_draft.accessory==1&&ws_draft.sunglasses==0);
 click(ws_choices[3]);ws_apply_style(NULL);tick();assert(saved.accessory==1&&saved.sunglasses==1&&!ws_opened);
 muse_style_ui_request(1,0);tick();click(ws_tabs[2]);assert(ws_draft.accessory==1&&ws_draft.sunglasses==1);
 click(ws_choices[2]);ws_close_style(NULL);assert(saved.accessory==1&&saved.sunglasses==1);
 puts("three_tabs=1 labels_fit=1 independent_glasses=1 save_cancel=1 preview_art_stubbed=1");lv_deinit();return 0;
}
