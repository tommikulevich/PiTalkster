#ifndef CONTROLS_H
#define CONTROLS_H

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

extern result_t controls_init( void );

#endif