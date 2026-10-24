/* Public-edition staging: local UI additions and/or privacy review; see source manifest. */
/* SPDX-License-Identifier: Apache-2.0 */
#pragma once
#include <stdbool.h>
typedef enum { PET_ANIM_NONE, PET_ANIM_COFFEE, PET_ANIM_CAKE, PET_ANIM_FULL, PET_ANIM_POOR, PET_ANIM_RICE, PET_ANIM_HOTPOT, PET_ANIM_CHICKEN, PET_ANIM_TEA } muse_pet_anim_kind_t;
typedef struct { float x,food_y,food_opa,mouth,happy; bool active,food,refuse; } muse_pet_anim_frame_t;
muse_pet_anim_frame_t muse_pet_anim_sample(muse_pet_anim_kind_t kind,float elapsed);
#include <stdint.h>
typedef struct {int x,y,w,h;uint32_t color;} muse_pet_food_rect_t;
int muse_pet_food_rects(muse_pet_anim_kind_t kind,muse_pet_food_rect_t *out,int capacity);
