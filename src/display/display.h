/**
 *******************************************************************************
 * @file    display.h
 * @brief   Display header file.
 *******************************************************************************
 */

#ifndef DISPLAY_H
#define DISPLAY_H

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

#define COLOR_STATUS    0x00CE79
#define COLOR_PROMPT    0x004FF4
#define COLOR_ANSWER    0x004E5F

/************
 * TYPEDEFS *
 ************/

 typedef struct {
    uint16_t curr_x;
    uint16_t curr_y;
} display_menu_t;

/******************************
 * GLOBAL FUNCTION PROTOTYPES *
 ******************************/

extern result_t display_menu_new_line( display_menu_t * m );
extern result_t display_menu_update_line( display_menu_t * m, const char * text, 
    uint32_t color );
extern result_t display_menu_append_text( display_menu_t * m, const char * text, 
    uint32_t color );

extern result_t display_init( void );

#ifdef __cplusplus
}
#endif

#endif /* DISPLAY_H */