/* SPDX-License-Identifier: MIT */

#include <stdint.h>
#include <zephyr/settings/settings.h>

/* DYA ZMK e5c9b691, app/src/activity.c exposes this state. Its public setters
 * persist changes, so the diagnostic changes RAM only after settings_load().
 * Keep this declaration in sync if the DYA activity implementation changes.
 */
extern struct activity_setting_state {
    uint32_t sleep_ms;
    uint32_t idle_ms;
} activity_settings;

static int sleep_test_settings_commit(void) {
    activity_settings.sleep_ms = 30000;
    activity_settings.idle_ms = 5000;
    return 0;
}

/* Commit runs after all saved settings have been loaded, even when this
 * handler has no stored keys. Never call a save API from this test hook.
 */
SETTINGS_STATIC_HANDLER_DEFINE(corcell_sleep_test, "corcell_sleep_test", NULL, NULL,
                               sleep_test_settings_commit, NULL);
