/**
 *******************************************************************************
 * @file    display.c
 * @brief   Display source file.
 *******************************************************************************
 */

/************
 * INCLUDES *
 ************/

#include <string.h>

#include "utils.h"

#include "display.h"
#include "display_hw.h"

/******************************
 * PRIVATE MACROS AND DEFINES *
 ******************************/

#define FONT_TEXT       12
#define LINE_HEIGHT     FONT_TEXT

#define CHAR_WIDTH      (FONT_TEXT / 2)
#define MAX_CHARS       (DISP_WIDTH / CHAR_WIDTH - 1)
#define MAX_LINE_LENGTH (DISP_WIDTH / CHAR_WIDTH)

/********************
 * STATIC FUNCTIONS *
 ********************/

static result_t wrap_write( display_menu_t * m, const char * text, uint32_t color ) {
    RETURN_IF_NULL(m);
    RETURN_IF_NULL(text);

    const char * p = text;
    char line_buf[MAX_LINE_LENGTH + 1];

    while( *p ) {
        if( m->curr_y >= DISP_HEIGHT - LINE_HEIGHT ) {
            RETURN_ON_ERROR( display_hw_clear() );
            m->curr_x = 0;
            m->curr_y = 0;
        }

        if( *p == '\n' || m->curr_x >= DISP_WIDTH ) {
            RETURN_ON_ERROR( display_menu_new_line(m) );
            if( *p == '\n' ) {
                p++;
                continue;
            }
        }

        while( *p == ' ' ) {
            RETURN_ON_ERROR( display_hw_write_string(m->curr_x, m->curr_y, " ", 2, 
                color, FONT_TEXT) );
            m->curr_x += CHAR_WIDTH;
            p++;
            if( m->curr_x >= DISP_WIDTH ) {
                display_menu_new_line(m);
            }
        }

        uint16_t word_len = 0;
        while( p[word_len] && p[word_len] != ' ' && p[word_len] != '\n' ) {
            word_len++;
        }

        if( m->curr_x + word_len * CHAR_WIDTH >= DISP_WIDTH && m->curr_x > 0 ) {
            RETURN_ON_ERROR( display_menu_new_line(m) );
        }

        memcpy(line_buf, p, word_len);
        line_buf[word_len] = '\0';

        RETURN_ON_ERROR( display_hw_write_string(m->curr_x, m->curr_y, line_buf, 
            word_len, color, FONT_TEXT) );
        m->curr_x += (uint16_t)(word_len * CHAR_WIDTH);

        p += word_len;
    }

    return RES_OK;
}

static result_t clear_line( uint16_t y ) {
    return display_hw_clear_area(0, y, DISP_WIDTH - 1, LINE_HEIGHT - 1);
}

/********************
 * GLOBAL FUNCTIONS *
 ********************/

result_t display_menu_new_line( display_menu_t * m ) {
    RETURN_IF_NULL(m);

    m->curr_x = 0;
    m->curr_y += LINE_HEIGHT;
    if( m->curr_y >= DISP_HEIGHT ) {
        m->curr_y = 0;
    }

    return clear_line(m->curr_y);
}

result_t display_menu_update_line( display_menu_t * m, const char * text, 
        uint32_t color) {
    RETURN_IF_NULL(m);
    RETURN_IF_NULL(text);

    m->curr_x = 0;
    RETURN_ON_ERROR( clear_line(m->curr_y) );

    return wrap_write(m, text, color);
}

result_t display_menu_append_text( display_menu_t * m, const char * text, 
        uint32_t color) {
    RETURN_IF_NULL(m);
    RETURN_IF_NULL(text);        
    return wrap_write(m, text, color);
}

result_t display_init( void ) {
    return display_hw_init();
}
