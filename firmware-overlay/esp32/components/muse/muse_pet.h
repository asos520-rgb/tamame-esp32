/* Public-edition staging: local UI additions and/or privacy review; see source manifest. */
/* SPDX-License-Identifier: Apache-2.0 */
#pragma once
#include "muse_pet_model.h"
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct { muse_pet_action_t action; muse_pet_result_t result; uint32_t fortune; } muse_pet_feedback_t;
void muse_pet_start(void);
bool muse_pet_request(muse_pet_action_t action);
bool muse_pet_snapshot(muse_pet_model_t *out);
bool muse_pet_feedback(muse_pet_feedback_t *out);
void muse_pet_conversation_completed(void);
#ifdef __cplusplus
}
#endif
