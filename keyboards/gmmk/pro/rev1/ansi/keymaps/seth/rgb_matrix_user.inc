RGB_MATRIX_EFFECT(jellybean_chevron)

#ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

#define NUM_COLORS 8 // should be a power of 2, other values cause the animation to drift
#define SCALE_FACTOR (256 / NUM_COLORS)
#define SPEED 6 // higher speed value means slower animation

static uint8_t colors[2][NUM_COLORS];
static uint8_t timer, next_j;
static uint32_t x_offs;

static void reset_j(uint8_t j) {
    colors[0][j] = rand() & 0xFF;
    colors[1][j] = qadd8(rand() & 0x7F, 0x80);
}

static void jellybean_chevron_init(effect_params_t* params) {
    timer = 0;
    x_offs = 0;
    next_j = 0;
    for (uint8_t i = 0; i < NUM_COLORS; i++)
        reset_j(i);
}

static bool jellybean_chevron_run(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    uint8_t i, j;
    for (i = led_min; i < led_max; i++) {
        j = ((abs8(g_led_config.point[i].y - k_rgb_matrix_center.y) + g_led_config.point[i].x - x_offs) / SCALE_FACTOR) & 0xFF;
        j %= NUM_COLORS;
        HSV hsv = {colors[0][j], colors[1][j], rgb_matrix_config.hsv.v};
        RGB rgb_conv = rgb_matrix_hsv_to_rgb(hsv);
        rgb_matrix_set_color(i, rgb_conv.r, rgb_conv.g, rgb_conv.b);
    }
    if (++timer == SPEED) {
        // next_j = x_offs % 256 == 0 ? NUM_COLORS / 2 : next_j; // reset position to counter drift
        if (++x_offs % SCALE_FACTOR == 0) {
            reset_j(next_j--);
            next_j %= NUM_COLORS;
        }
        timer = 0;
    }
    return rgb_matrix_check_finished_leds(led_max);
}

static bool jellybean_chevron(effect_params_t* params) {
    if (params->init) jellybean_chevron_init(params);
        return jellybean_chevron_run(params);
}

#endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
