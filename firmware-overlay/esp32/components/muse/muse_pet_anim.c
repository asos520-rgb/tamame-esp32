/* Public-edition staging: local UI additions and/or privacy review; see source manifest. */
/* SPDX-License-Identifier: Apache-2.0 */
#include "muse_pet_anim.h"
#include <math.h>
static float smooth(float x){if(x<0)x=0;if(x>1)x=1;return x*x*(3-2*x);}
muse_pet_anim_frame_t muse_pet_anim_sample(muse_pet_anim_kind_t kind,float t) {
    muse_pet_anim_frame_t f={0};
    if(!isfinite(t)||t<0||kind==PET_ANIM_NONE)return f;
    if(kind==PET_ANIM_FULL||kind==PET_ANIM_POOR){
        f.active=t<1.2f;f.refuse=f.active;
        /* Full stomach: absolutely stationary; only insufficient coins shakes. */
        if(kind==PET_ANIM_POOR&&f.active)f.x=sinf(t*6.2831853f*3)*5*(1-t/1.2f);
        return f;
    }
    if(t>=6.f)return f;
    f.active=true;f.food=t<3.8f;f.food_opa=t<3.4f?1:t<3.8f?(3.8f-t)/.4f:0;
    f.food_y=-70+70*smooth(t/.6f);
    if(t<.6f)f.x=0;
    else if(t<1.4f)f.x=32*smooth((t-.6f)/.8f);
    else if(t<5)f.x=32;
    else f.x=32*(1-smooth((t-5)/1.f));
    if(t>=1.4f&&t<4)f.mouth=fmodf(t-1.4f,.44f)<.20f?.75f:0;
    if(t>=4&&t<5)f.happy=1;
    return f;
}
int muse_pet_food_rects(muse_pet_anim_kind_t kind,muse_pet_food_rect_t *out,int capacity) {
    static const muse_pet_food_rect_t coffee[]={
        {10,0,2,5,0xB5A7C2},{17,2,2,4,0xB5A7C2},{24,0,2,5,0xB5A7C2},
        {6,9,21,16,0xF4F0E7},{8,8,17,4,0x704128},{27,12,7,9,0xF4F0E7},
        {29,14,3,5,0x000000},{10,13,3,8,0xC8A888},{4,26,29,2,0xB58AF5}};
    static const muse_pet_food_rect_t cake[]={
        {5,11,25,15,0xE9BF7D},{3,7,29,5,0x7D4525},{5,25,25,3,0x925D32},
        {26,12,4,13,0xC38D52},{7,14,3,8,0xFFE3AE},{6,6,23,2,0xA96635}};
    const muse_pet_food_rect_t *r=kind==PET_ANIM_COFFEE?coffee:cake;
    int n=kind==PET_ANIM_COFFEE?9:6;
    if(!out||capacity<n||(kind!=PET_ANIM_COFFEE&&kind!=PET_ANIM_CAKE))return 0;
    for(int i=0;i<n;i++)out[i]=r[i];
    return n;
}
