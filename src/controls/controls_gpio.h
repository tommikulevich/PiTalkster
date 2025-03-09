/**
 *******************************************************************************
 * @file    controls_gpio.h
 * @brief   Controls GPIO header file. 
 *          Includes button GPIO numbers.
 *******************************************************************************
 */

#ifndef CONTROLS_GPIO_H
#define CONTROLS_GPIO_H

#ifdef __cplusplus
extern "C" {
#endif

/************
 * TYPEDEFS *
 ************/

typedef enum {
    BUTTON_UP_GPIO      = 22,
    BUTTON_OK_GPIO      = 5,
    BUTTON_DOWN_GPIO    = 6
} button_gpio_t;

#ifdef __cplusplus
}
#endif

#endif /* CONTROLS_GPIO_H */
