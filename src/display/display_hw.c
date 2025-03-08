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

/******************************
 * PRIVATE MACROS AND DEFINES *
 ******************************/

#define COLOR_BACKGROUND    0x000000

/********************
 * GLOBAL FUNCTIONS *
 ********************/

result_t display_hw_turn_on( void ) {
    RETURN_ERROR_IF( st7789_basic_display_on() != 0, RES_ERR_GENERIC );
    return RES_OK;
}

result_t display_hw_turn_off( void ) {
    RETURN_ERROR_IF( st7789_basic_display_off() != 0, RES_ERR_GENERIC );
    return RES_OK;
}

result_t display_hw_clear( void ) {
    RETURN_ERROR_IF( st7789_basic_clear() != 0, RES_ERR_GENERIC );
    return RES_OK;
}

result_t display_hw_clear_area( uint16_t x, uint16_t y, uint16_t w, uint16_t h ) {
    RETURN_ERROR_IF( st7789_basic_rect(x, y,
        (uint16_t)(x + w - 1), (uint16_t)(y + h - 1), COLOR_BACKGROUND) != 0, 
        RES_ERR_GENERIC );
    return RES_OK;
}

result_t display_hw_write_string( uint16_t x, uint16_t y, char * str, uint16_t len, 
        uint32_t color, uint16_t font ) {
    RETURN_ERROR_IF( st7789_basic_string(x, y, str, len, color, (st7789_font_t)font) != 0, 
        RES_ERR_GENERIC );
    return RES_OK;
}

result_t display_hw_init( void ) {
    if( st7789_basic_init() == 0 ) {
        RETURN_ON_ERROR( display_hw_turn_on() );
        RETURN_ON_ERROR( display_hw_clear() );
        return RES_OK;
    }

    return RES_ERR_NOT_READY;
}
