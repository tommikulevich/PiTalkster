/**
 *******************************************************************************
 * @file    display.c
 * @brief   Display source file.
 *          Includes thread with event-driven part.
 *******************************************************************************
 */

/************
 * INCLUDES *
 ************/

#include "utils.h"

#include "display.h"
#include "display_hw.h"

/********************
 * GLOBAL FUNCTIONS *
 ********************/

result_t display_init( void ) {
    return display_hw_init();
}
