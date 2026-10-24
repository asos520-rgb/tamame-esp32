/* Public-edition staging: local UI additions and/or privacy review; see source manifest. */
#pragma once
#include "muse_state.h"
typedef struct {
    uint32_t rgb;
    uint8_t opacity;
} muse_ring_effect_t;
/* Display ring only. Diagnostics override daily animation; no LED hardware. */
muse_ring_effect_t muse_ring_effect(muse_mode_t mode,float seconds,float mode_seconds,
                                    float level,float happy,bool pairing,bool error,
                                    bool connected_flash);
