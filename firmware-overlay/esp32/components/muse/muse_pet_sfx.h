/* Public-edition staging: local UI additions and/or privacy review; see source manifest. */
/* SPDX-License-Identifier: Apache-2.0 */
#pragma once
#include <stdint.h>
#include <stddef.h>
typedef enum { PET_SFX_POP=1, PET_SFX_COIN, PET_SFX_CHEW, PET_SFX_CARD, PET_SFX_REFUSE, PET_SFX_REWARD } muse_pet_sfx_t;
size_t muse_pet_sfx_length(muse_pet_sfx_t effect);
int16_t muse_pet_sfx_sample(muse_pet_sfx_t effect,size_t frame);
