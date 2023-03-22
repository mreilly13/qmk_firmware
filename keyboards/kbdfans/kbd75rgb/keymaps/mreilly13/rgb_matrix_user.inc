RGB_MATRIX_EFFECT(backlit_heatmap)
#ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

#define DELAY_MS 25
#define SPREAD 40
#define AREA_LIMIT 16
#define LED_DISTANCE(led_a, led_b) sqrt16(((int16_t)(led_a.x - led_b.x) * (int16_t)(led_a.x - led_b.x)) + ((int16_t)(led_a.y - led_b.y) * (int16_t)(led_a.y - led_b.y)))
#define BACKLIGHT_LEVEL 0x0F

void process_rgb_matrix_backlit_heatmap(uint8_t row, uint8_t col) {
    if (g_led_config.matrix_co[row][col] == NO_LED) return; // skip as pressed key doesn't have an led position
    for (uint8_t i_row = 0; i_row < MATRIX_ROWS; i_row++) {
        for (uint8_t i_col = 0; i_col < MATRIX_COLS; i_col++) {
            if (g_led_config.matrix_co[i_row][i_col] == NO_LED) continue; // skip as target key doesn't have an led position
            if (i_row == row && i_col == col) {
                g_rgb_frame_buffer[row][col] = qadd8(g_rgb_frame_buffer[row][col], 32);
            } else {
                uint8_t distance = LED_DISTANCE(g_led_config.point[g_led_config.matrix_co[row][col]], g_led_config.point[g_led_config.matrix_co[i_row][i_col]]);
                if (distance <= SPREAD) {
                    uint8_t amount = qsub8(SPREAD, distance);
                    if (amount > AREA_LIMIT) {
                        amount = AREA_LIMIT;
                    }
                    g_rgb_frame_buffer[i_row][i_col] = qadd8(g_rgb_frame_buffer[i_row][i_col], amount);
                }
            }
        }
    }
}

// A timer to track the last time we decremented all heatmap values.
static uint16_t heatmap_decrease_timer;
// Whether we should decrement the heatmap values during the next update.
static bool decrease_heatmap_values;

bool backlit_heatmap(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);

    if (params->init) {
        rgb_matrix_set_color_all(BACKLIGHT_LEVEL, BACKLIGHT_LEVEL, BACKLIGHT_LEVEL);
        memset(g_rgb_frame_buffer, 0, sizeof g_rgb_frame_buffer);
    }

    // The heatmap animation might run in several iterations depending on
    // `RGB_MATRIX_LED_PROCESS_LIMIT`, therefore we only want to update the
    // timer when the animation starts.
    if (params->iter == 0) {
        decrease_heatmap_values = timer_elapsed(heatmap_decrease_timer) >= DELAY_MS;

        // Restart the timer if we are going to decrease the heatmap this frame.
        if (decrease_heatmap_values) {
            heatmap_decrease_timer = timer_read();
        }
    }

    // Render heatmap & decrease
    uint8_t count = 0;
    for (uint8_t row = 0; row < MATRIX_ROWS && count < RGB_MATRIX_LED_PROCESS_LIMIT; row++) {
        for (uint8_t col = 0; col < MATRIX_COLS && RGB_MATRIX_LED_PROCESS_LIMIT; col++) {
            if (g_led_config.matrix_co[row][col] >= led_min && g_led_config.matrix_co[row][col] < led_max) {
                count++;
                uint8_t val = g_rgb_frame_buffer[row][col];
                if (!HAS_ANY_FLAGS(g_led_config.flags[g_led_config.matrix_co[row][col]], params->flags)) continue;
                HSV hsv = {170 - qsub8(val, 85), rgb_matrix_config.hsv.s, scale8((qadd8(170, val) - 170) * 3, rgb_matrix_config.hsv.v)};
                RGB rgb = rgb_matrix_hsv_to_rgb(hsv);
                rgb_matrix_set_color(g_led_config.matrix_co[row][col], MAX(BACKLIGHT_LEVEL, rgb.r), MAX(BACKLIGHT_LEVEL, rgb.g), MAX(BACKLIGHT_LEVEL, rgb.b));

                if (decrease_heatmap_values) {
                    g_rgb_frame_buffer[row][col] = qsub8(val, 1);
                }
            }
        }
    }

    return rgb_matrix_check_finished_leds(led_max);
}

#    endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
