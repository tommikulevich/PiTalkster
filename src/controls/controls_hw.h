#ifndef CONTROLS_HW_H
#define CONTROLS_HW_H

#include "utils.h"

typedef enum {
    BUTTON_UP_GPIO      = 22,
    BUTTON_OK_GPIO      = 5,
    BUTTON_DOWN_GPIO    = 6
} button_gpio_t;

static inline const char* button_gpio_enum_to_string( button_gpio_t button_gpio ) {
    switch( button_gpio ) {
        case BUTTON_UP_GPIO:    return "BUTTON_UP";
        case BUTTON_OK_GPIO:    return "BUTTON_OK";
        case BUTTON_DOWN_GPIO:  return "BUTTON_DOWN";

        default:                return "BUTTON_UNDEFINED";
    }
}

typedef struct {
    void (*action_on_press_button)(button_gpio_t);
} action_wrapper_t;

extern result_t controls_hw_init( const action_wrapper_t * action_wrapper );

#endif
