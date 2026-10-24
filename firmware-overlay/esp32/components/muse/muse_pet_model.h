/* Public-edition staging: local UI additions and/or privacy review; see source manifest. */
/* SPDX-License-Identifier: Apache-2.0 */
#pragma once
#include <stdbool.h>
#include <stdint.h>
#define MUSE_PET_VERSION 1u
#define MUSE_PET_DAILY_CAP 30u
#define MUSE_PET_FOOD_COST 5u
#define MUSE_PET_FORTUNES 20u
#define MUSE_PET_MIN_TIME 1704067200LL
/* Integer millipoints preserve decay remainder across interactions/reboots. */
typedef struct {
    uint32_t version, coins;
    int32_t reward_day, fortune_day;
    uint32_t daily_pets, fortune;
    int32_t hunger, mood, energy;
    int64_t last_time;
    uint32_t sfx;
    uint32_t reserved;
} muse_pet_model_t;
typedef enum { PET_TOUCH, PET_COFFEE, PET_CAKE, PET_FORTUNE, PET_CONVERSATION, PET_SFX_OFF, PET_SFX_ON, PET_RICE, PET_HOTPOT, PET_CHICKEN, PET_TEA } muse_pet_action_t;
typedef enum { PET_OK, PET_COIN, PET_CAP, PET_TIME, PET_FULL, PET_POOR, PET_FORTUNE_NEW, PET_FORTUNE_SAME, PET_STORAGE, PET_BUSY } muse_pet_result_t;
void muse_pet_model_init(muse_pet_model_t *s, int64_t now);
bool muse_pet_model_valid(const muse_pet_model_t *s);
void muse_pet_model_decay(muse_pet_model_t *s, int64_t now);
void muse_pet_model_decay_uncalibrated(muse_pet_model_t *s, uint32_t seconds);
muse_pet_result_t muse_pet_model_apply(muse_pet_model_t *s, muse_pet_action_t action, int64_t now, uint32_t random);
const char *muse_pet_fortune(unsigned index);

typedef struct {const char *name;unsigned cost,fullness;} muse_pet_food_t;
const muse_pet_food_t *muse_pet_food(muse_pet_action_t action);
const char *muse_pet_fortune_grade(unsigned index);
