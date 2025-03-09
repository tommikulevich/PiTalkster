/**
 *******************************************************************************
 * @file    controls_hw.h
 * @brief   Controls HW header file.
 *******************************************************************************
 */

#ifndef CONTROLS_HW_H
#define CONTROLS_HW_H

#ifdef __cplusplus
extern "C" {
#endif

/************
 * INCLUDES *
 ************/

#include "utils.h"

#include "controls_gpio.h"

/*******************************
 * TYPEDEFS AND STATIC INLINES *
 *******************************/

typedef void (*button_handler_t)(button_gpio_t gpio);

static inline const char* button_gpio_enum_to_string( button_gpio_t button_gpio ) {
    switch( button_gpio ) {
        case BUTTON_UP_GPIO:    return "BUTTON_UP";
        case BUTTON_OK_GPIO:    return "BUTTON_OK";
        case BUTTON_DOWN_GPIO:  return "BUTTON_DOWN";

        default:                return "BUTTON_UNDEFINED";
    }
}

/******************************
 * GLOBAL FUNCTION PROTOTYPES *
 ******************************/

extern result_t controls_hw_init_button( button_gpio_t gpio, 
    button_handler_t handler );

#ifdef __cplusplus
}
#endif

#endif /* CONTROLS_HW_H */
