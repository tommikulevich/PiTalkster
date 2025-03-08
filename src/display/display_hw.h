/**
 *******************************************************************************
 * @file    display_hw.h
 * @brief   Display HW header file.
 *******************************************************************************
 */

#ifndef DISPLAY_HW_H
#define DISPLAY_HW_H

#ifdef __cplusplus
extern "C" {
#endif

/************
 * INCLUDES *
 ************/

#include "utils.h"

/**********************
 * MACROS AND DEFINES *
 **********************/

#define DISP_WIDTH  240
#define DISP_HEIGHT 240

/******************************
 * GLOBAL FUNCTION PROTOTYPES *
 ******************************/

extern result_t display_hw_turn_on( void );
extern result_t display_hw_turn_off( void );

extern result_t display_hw_clear( void );
extern result_t display_hw_clear_area( uint16_t x, uint16_t y, 
    uint16_t w, uint16_t h );

extern result_t display_hw_write_string( uint16_t x, uint16_t y, 
    char * str, uint16_t len, uint32_t color, uint16_t font );

extern result_t display_hw_init( void );

#ifdef __cplusplus
}
#endif

#endif /* DISPLAY_HW_H */