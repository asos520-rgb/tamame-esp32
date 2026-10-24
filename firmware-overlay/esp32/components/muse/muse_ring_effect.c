/* Public-edition staging: local UI additions and/or privacy review; see source manifest. */
#include "muse_ring_effect.h"
#include <math.h>
#define TAU 6.28318530718f
static float unit(float x) {return !isfinite(x)||x<0?0:x>1?1:x;}
static float breath(float t,float period) {return .5f-.5f*cosf(TAU*fmodf(t,period)/period);}
muse_ring_effect_t muse_ring_effect(muse_mode_t mode,float seconds,float mode_seconds,
                                    float level,float happy,bool pairing,bool error,
                                    bool connected_flash)
{
    float t=isfinite(seconds)?fmaxf(0,seconds):0;
    float gain=0;
    uint32_t color=0;
    if(mode==MUSE_MODE_OFF)return (muse_ring_effect_t){0,0};
    if(error||mode==MUSE_MODE_ERROR) {
        color=0xFF4040;gain=fmodf(t,.4f)<.2f?1:0;
    } else if(pairing) {
        color=0xFF9A32;gain=.12f+.88f*breath(t,2.0f);
    } else if(connected_flash) {
        color=0x4EE879;gain=1;
    } else if(unit(happy)>.05f) {
        color=0xFF6060;gain=unit(happy);
    } else switch(mode) {
    case MUSE_MODE_LISTENING:color=0x50E5EE;gain=.16f+.84f*breath(t,1.4f);break;
    case MUSE_MODE_THINKING: {float pulse=breath(t,1.0f);color=0xFFB547;gain=.12f+.88f*pulse*pulse;break;}
    case MUSE_MODE_SPEAKING:color=0x9DE8C8;gain=.06f+.94f*unit(level);break;
    case MUSE_MODE_BOOT:color=0xB58AF5;gain=.6f*unit(mode_seconds/1.2f);break;
    default:color=0xB58AF5;gain=.12f+.58f*breath(t,4.0f);break;
    }
    return (muse_ring_effect_t){color,(uint8_t)lroundf(unit(gain)*255)};
}
