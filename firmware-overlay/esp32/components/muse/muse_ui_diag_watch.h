/* Public-edition staging: local UI additions and/or privacy review; see source manifest. */
/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* Pure progress detector. Milliseconds and sequence numbers may wrap. */
#pragma once
#include <stdbool.h>
#include <stdint.h>
typedef struct {
    uint32_t sequence, changed_ms;
    bool initialized;
} muse_ui_diag_watch_t;
static inline bool muse_ui_diag_stalled(muse_ui_diag_watch_t *w, uint32_t seq, uint32_t now_ms)
{
    if (!w->initialized || w->sequence != seq) {
        w->initialized = true;
        w->sequence = seq;
        w->changed_ms = now_ms;
    }
    return (uint32_t)(now_ms - w->changed_ms) >= 6000u;
}
