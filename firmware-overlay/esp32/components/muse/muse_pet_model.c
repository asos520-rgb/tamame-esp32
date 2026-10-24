#include "muse_i18n.h"
/* Public-edition staging: local UI additions and/or privacy review; see source manifest. */
/* SPDX-License-Identifier: Apache-2.0 */
#include "muse_pet_model.h"
#include <limits.h>
#include <string.h>
static int32_t plus(int32_t value, int32_t amount) { return value+amount>100000?100000:value+amount; }
static int32_t decay(int32_t value, int64_t seconds, int per_hour) {
    int64_t loss=seconds*per_hour/3600;
    return loss>=value?0:value-(int32_t)loss;
}
void muse_pet_model_init(muse_pet_model_t *s,int64_t now) {
    memset(s,0,sizeof(*s));s->version=MUSE_PET_VERSION;s->reward_day=-1;s->fortune_day=-1;
    s->hunger=70000;s->mood=75000;s->energy=75000;s->sfx=1;
    s->last_time=now>=MUSE_PET_MIN_TIME?now:0;
}
bool muse_pet_model_valid(const muse_pet_model_t *s) {
    return s->version==MUSE_PET_VERSION&&s->coins<=1000000&&s->daily_pets<=30&&s->fortune<20&&
        s->reward_day>=-1&&s->fortune_day>=-1&&s->hunger>=0&&s->hunger<=100000&&
        s->mood>=0&&s->mood<=100000&&s->energy>=0&&s->energy<=100000&&s->sfx<=1&&s->reserved<=30u*86400&&s->last_time>=0;
}
void muse_pet_model_decay(muse_pet_model_t *s,int64_t now) {
    if(now<MUSE_PET_MIN_TIME)return;
    int32_t day=(int32_t)((now+8*3600)/86400);
    if(s->reward_day==-1&&s->daily_pets)s->reward_day=day;
    if(s->fortune_day==0)s->fortune_day=day;
    if(!s->last_time){s->last_time=now;s->reserved=0;return;}
    if(now<=s->last_time)return; /* clock rollback must not grant anything */
    int64_t accounted=now-s->last_time;
    if(accounted>s->reserved)accounted=s->reserved;
    s->last_time+=accounted;s->reserved-=(uint32_t)accounted;
    int64_t seconds=now-s->last_time;
    if(seconds>30LL*86400)seconds=30LL*86400;
    /* Only whole 36-second quanta: exact millipoints for all three rates.
     * Keep unconsumed seconds in last_time, so frequent touches cannot stop decay. */
    seconds=seconds/36*36;
    if(!seconds)return;
    s->hunger=decay(s->hunger,seconds,2000);
    s->mood=decay(s->mood,seconds,1000);
    s->energy=decay(s->energy,seconds,1500);
    s->last_time+=(now-s->last_time>30LL*86400)?now-s->last_time:seconds;
}
void muse_pet_model_decay_uncalibrated(muse_pet_model_t *s,uint32_t seconds) {
    seconds=seconds/36*36;if(seconds>30u*86400)seconds=30u*86400;
    s->hunger=decay(s->hunger,seconds,2000);s->mood=decay(s->mood,seconds,1000);
    s->energy=decay(s->energy,seconds,1500);
    if(s->reserved+seconds>30u*86400)s->reserved=30u*86400;else s->reserved+=seconds;
}
muse_pet_result_t muse_pet_model_apply(muse_pet_model_t *s,muse_pet_action_t action,int64_t now,uint32_t random) {
    muse_pet_model_decay(s,now);
    bool clock=now>=MUSE_PET_MIN_TIME;
    int32_t day=clock?(int32_t)((now+8*3600)/86400):-1;
    if(action==PET_TOUCH) {
        s->mood=plus(s->mood,3000);
        if(clock&&day<s->reward_day)return PET_TIME;
        if(clock&&day>s->reward_day){
            /* First sync carries provisional rewards into the current day. */
            if(s->reward_day>=0)s->daily_pets=0;
            s->reward_day=day;
        }
        if(s->daily_pets>=30)return PET_CAP;
        s->daily_pets++;if(s->coins<1000000)s->coins++;return PET_COIN;
    }
    const muse_pet_food_t *food=muse_pet_food(action);
    if(food) {
        if(s->hunger>=100000)return PET_FULL;
        if(s->coins<food->cost)return PET_POOR;
        s->coins-=food->cost;s->hunger=plus(s->hunger,(int)food->fullness*1000);s->mood=plus(s->mood,8000);
        return PET_OK;
    }
    if(action==PET_FORTUNE) {
        if(!clock){
            if(s->fortune_day!=-1)return PET_FORTUNE_SAME;
            s->fortune_day=0;s->fortune=random%MUSE_PET_FORTUNES;return PET_FORTUNE_NEW;
        }
        if(s->fortune_day==0){s->fortune_day=day;return PET_FORTUNE_SAME;}
        if(day<=s->fortune_day)return PET_FORTUNE_SAME;
        s->fortune_day=day;s->fortune=random%MUSE_PET_FORTUNES;return PET_FORTUNE_NEW;
    }
    if(action==PET_CONVERSATION){s->energy=plus(s->energy,10000);return PET_OK;}
    if(action==PET_SFX_OFF||action==PET_SFX_ON){s->sfx=action==PET_SFX_ON;return PET_OK;}
    return PET_BUSY;
}
const char *muse_pet_fortune(unsigned index) {
    static const char *const cards[]={
        MUSE_KEY_s211, MUSE_KEY_s212,
        MUSE_KEY_s213, MUSE_KEY_s207,
        MUSE_KEY_s206, MUSE_KEY_s204,
        MUSE_KEY_s208, MUSE_KEY_s205,
        MUSE_KEY_s209, MUSE_KEY_s203,
        MUSE_KEY_s217, MUSE_KEY_s215,
        MUSE_KEY_s216, MUSE_KEY_s214,
        MUSE_KEY_s218, MUSE_KEY_s220,
        MUSE_KEY_s222, MUSE_KEY_s219,
        MUSE_KEY_s223, MUSE_KEY_s221
    };
    return muse_i18n_text(cards[index%MUSE_PET_FORTUNES]);
}

const muse_pet_food_t *muse_pet_food(muse_pet_action_t a) {
    static const muse_pet_food_t foods[]={{MUSE_KEY_s210,3,15},{MUSE_KEY_s227,5,25},{MUSE_KEY_s224,7,40},{MUSE_KEY_s225,10,60},{MUSE_KEY_s228,6,35},{MUSE_KEY_s226,4,20}};
    int i=a==PET_COFFEE?0:a==PET_CAKE?1:a==PET_RICE?2:a==PET_HOTPOT?3:a==PET_CHICKEN?4:a==PET_TEA?5:-1;
    return i<0?NULL:&foods[i];
}
const char *muse_pet_fortune_grade(unsigned i) {
    static const char *grades[]={MUSE_KEY_s200,MUSE_KEY_s200,MUSE_KEY_s200,MUSE_KEY_s198,MUSE_KEY_s199,MUSE_KEY_s198,MUSE_KEY_s199,MUSE_KEY_s198,MUSE_KEY_s199,MUSE_KEY_s199,MUSE_KEY_s201,MUSE_KEY_s201,MUSE_KEY_s201,MUSE_KEY_s201,MUSE_KEY_s201,MUSE_KEY_s202,MUSE_KEY_s202,MUSE_KEY_s202,MUSE_KEY_s202,MUSE_KEY_s202};
    return muse_i18n_text(grades[i%20]);
}
