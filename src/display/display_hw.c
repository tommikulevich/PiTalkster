/**
 *******************************************************************************
 * @file    display_hw.c
 * @brief   Display HW source file.
 *          Interaction with physical display using external ST7789 library.
 *******************************************************************************
 */

/************
 * INCLUDES *
 ************/

#include "utils.h"

#include "driver_st7789_basic.h"

/********************
 * GLOBAL FUNCTIONS *
 ********************/

result_t display_hw_init( void ) {
    if( st7789_basic_init() == 0 ) {
        return st7789_basic_clear();
    }

    return RES_ERR_NOT_READY;
}
