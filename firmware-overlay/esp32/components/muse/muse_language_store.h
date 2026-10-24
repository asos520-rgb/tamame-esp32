/* SPDX-License-Identifier: Apache-2.0 */
#pragma once
#include "esp_err.h"
#include "muse_i18n.h"
/* Call after NVS initialization and before UI construction. Never erases NVS. */
esp_err_t muse_language_load(void);
/* Saves only the public UI preference. Reboot applies it to all static labels. */
esp_err_t muse_language_save(unsigned language);
