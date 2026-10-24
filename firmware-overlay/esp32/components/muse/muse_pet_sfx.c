/* Public-edition staging: local UI additions and/or privacy review; see source manifest. */
/* SPDX-License-Identifier: Apache-2.0 */
#include "muse_pet_sfx.h"
#include <math.h>
#define RATE 16000u
#define TAU 6.28318530718f
size_t muse_pet_sfx_length(muse_pet_sfx_t e) {
    const unsigned ms[]={0,85,210,180,160,180,325};
    return (unsigned)e<7?RATE*ms[e]/1000:0;
}
int16_t muse_pet_sfx_sample(muse_pet_sfx_t e,size_t i) {
    size_t n=muse_pet_sfx_length(e);if(!n||i>=n)return 0;
    if(e==PET_SFX_REWARD){
        if(i<RATE*85/1000)return muse_pet_sfx_sample(PET_SFX_POP,i);
        if(i<RATE*115/1000)return 0;
        return muse_pet_sfx_sample(PET_SFX_COIN,i-RATE*115/1000);
    }
    float t=(float)i/RATE,x=(float)i/n;
    float env=sinf(3.14159265359f*x);env*=env;
    float y=0;
    if(e==PET_SFX_POP)y=sinf(TAU*(1000*t-3500*t*t))*(1-x);
    else if(e==PET_SFX_COIN){float f=x<.42f?1320:1760;y=(sinf(TAU*f*t)+.25f*sinf(TAU*2*f*t))*(1-x);}
    else if(e==PET_SFX_CHEW){
        float open=x<.5f?1:x<.7f?( .7f-x)/.2f:0;
        float ah=.5f*sinf(TAU*180*t)+.3f*sinf(TAU*600*t)+.2f*sinf(TAU*980*t);
        float mm=.8f*sinf(TAU*150*t)+.2f*sinf(TAU*300*t);y=open*ah+(1-open)*mm;
    }
    else if(e==PET_SFX_CARD){uint32_t v=(uint32_t)i*747796405u+2891336453u;v=((v>>((v>>28)+4))^v)*277803737u;v=(v>>22)^v;y=((int)(v&65535)-32768)/32768.0f*.7f;}
    else if(e==PET_SFX_REFUSE)y=.7f*sinf(TAU*(180*t-200*t*t))+.2f*sinf(TAU*360*t);
    return (int16_t)(y*env*5500);
}
