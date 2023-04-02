RGB_MATRIX_EFFECT(backlit_heatmap)
#ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

#define BACKLIGHT 25 // minimum lighting level
#define COLOR_WIDTH 125 // color width; maximum value added to base lighting level per color
#define MAX_TEMP (3 * COLOR_WIDTH) // maximum temperature
#define KEYPRESS_TEMP 50 // how much pressing a key increases the temp by
#define CONDUCTIVITY 50 // determines how far heat spreads on keypress; larger is more spread
#define COOL_FACTOR 30 // determines how fast keys cool down; larger is slower cooling
#define HEAT 1
#define COOL -1
#define LED_DISTANCE(led_a, led_b) sqrt16(((int16_t)(led_a.x - led_b.x) * (int16_t)(led_a.x - led_b.x)) + ((int16_t)(led_a.y - led_b.y) * (int16_t)(led_a.y - led_b.y)))

static uint16_t temps[MATRIX_ROWS][MATRIX_COLS];
static uint8_t frame;

static uint8_t isqrt(uint8_t n) {
    uint8_t x = 1;
    while ((x + 1) * (x + 1) <= n) x++;
    return x;
}

static RGB temp_to_rgb(uint16_t temp) {
    uint8_t r = BACKLIGHT + MAX(0, COLOR_WIDTH - abs((3 * COLOR_WIDTH) - temp));
    uint8_t g = BACKLIGHT + MAX(0, COLOR_WIDTH - abs((2 * COLOR_WIDTH) - temp));
    uint8_t b = BACKLIGHT + MAX(0, COLOR_WIDTH - abs((1 * COLOR_WIDTH) - temp));
    RGB rgb = {g, r, b};
    return rgb;
}

static void change_temp_by(uint8_t row, uint8_t col, uint16_t temp, int mode) {
    if (mode == HEAT && temps[row][col] + temp >= MAX_TEMP) temps[row][col] = MAX_TEMP;
    else if (mode == COOL && temp >= temps[row][col]) temps[row][col] = 0;
    else temps[row][col] += temp * mode;
}

void process_keypress(uint8_t row, uint8_t col) {
    uint8_t i = g_led_config.matrix_co[row][col];
    if (i == NO_LED) return; // skip as pressed key doesn't have an led position
    change_temp_by(row, col, KEYPRESS_TEMP, HEAT);
    uprintf("key %d, temp %d\n", g_led_config.matrix_co[row][col], temps[row][col]);
    uint8_t j, d;
    for (row = 0; row < MATRIX_ROWS; row++) {
        for (col = 0; col < MATRIX_COLS; col++) {
            j = g_led_config.matrix_co[row][col];
            if (j == NO_LED || j == i) continue; // skip as this key doesn't have an led position, or is the source key
            d = LED_DISTANCE(g_led_config.point[i], g_led_config.point[j]);
            if (d < CONDUCTIVITY) {
                change_temp_by(row, col, KEYPRESS_TEMP - (KEYPRESS_TEMP * d / CONDUCTIVITY), HEAT);
            }
        }
    }
}

static void backlit_heatmap_init(effect_params_t* params) {
    frame = 0;
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            temps[row][col] = 0;
        }
    }
}

static bool backlit_heatmap_run(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    if (++frame == COOL_FACTOR) {
        frame = 0;
        uint8_t i;
        for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
            for (uint8_t col = 0; col < MATRIX_COLS; col++) {
                i = g_led_config.matrix_co[row][col];
                if (i == NO_LED) continue; // skip as key doesn't have an led position
                change_temp_by(row, col, isqrt(temps[row][col]), COOL);
                RGB rgb = temp_to_rgb(temps[row][col]);
                rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
            }
        }
    }
    return rgb_matrix_check_finished_leds(led_max);
}

static bool backlit_heatmap(effect_params_t* params) {
    if (params->init) backlit_heatmap_init(params);
    return backlit_heatmap_run(params);
}

#    endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
