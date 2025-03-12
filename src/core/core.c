/**
 *******************************************************************************
 * @file    core.c
 * @brief   Core source file.
 *          Includes thread with event-driven part.
 *          Coordinates the display and buttons.
 *******************************************************************************
 */

/************
 * INCLUDES *
 ************/

#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include "utils.h"

#include "core.h"
#include "event_broker.h"
#include "controls.h"
#include "controls_gpio.h"
#include "display.h"

/******************************
 * PRIVATE MACROS AND DEFINES *
 ******************************/

#define EOF_POS -1

/********************
 * PRIVATE TYPEDEFS *
 ********************/

typedef enum {
    CORE_STATE_WAIT_FOR_START = 0,
    CORE_STATE_REC_PROCESSING,
    CORE_STATE_STT_PROCESSING,
    CORE_STATE_LLM_PROCESSING
} core_state_t;

typedef struct {
    FILE * last_answer_file; 
    long page_pos;   
    long page_pos_history[64];
    int  page_pos_history_idx;
} answer_context_t;

typedef struct {
    core_state_t state;
    display_menu_t menu;
    answer_context_t ans;    

    uint64_t last_time_pressed_ok_us;
} core_context_t;

/********************
 * STATIC FUNCTIONS *
 ********************/

static void show_welcome_text( display_menu_t * menu ) {
    display_menu_clear(menu);
    display_menu_append_text(menu, "Welcome!\n", COLOR_TIP);
    display_menu_append_text(menu, "> To start recording, press \"O\".\n", COLOR_TIP);
    display_menu_append_text(menu, "> To stop any processing, press \"O\" again.\n", COLOR_TIP);
}

static void show_final_text( display_menu_t * menu ) {
    display_menu_clear(menu);
    display_menu_append_text(menu, "Do you want to show full prompt with answer?\n", COLOR_TIP);
    display_menu_append_text(menu, "> To go to the view and then scroll, press \"<\" or \">\".\n", COLOR_TIP);
}

static void update_last_time_pressed( uint64_t * last_time_pressed ) {
    *last_time_pressed = get_current_time_us();
}

// === EVENTS ===

static void rec_request_event_publish( void ) {    
    event_t event = STRUCT_INIT_ALL_ZEROS;
    result_t res = event_create(
        COMPONENT_CORE_DISP, COMPONENT_AUDIO_INPUT,
        EVENT_REC_REQUEST, 
        NULL, 0,
        &event);

    if( res == RES_OK ) {
        broker_publish(&event);
    }
}

static void rec_stop_event_publish( void ) {
    event_t event = STRUCT_INIT_ALL_ZEROS;
    result_t res = event_create(
        COMPONENT_CORE_DISP, COMPONENT_AUDIO_INPUT,
        EVENT_REC_STOP, 
        NULL, 0,
        &event);

    if( res == RES_OK ) {
        broker_publish(&event);
    }
}

static void stt_stop_event_publish( void ) {
    event_t event = STRUCT_INIT_ALL_ZEROS;
    result_t res = event_create(
        COMPONENT_CORE_DISP, COMPONENT_STT,
        EVENT_STT_STOP, 
        NULL, 0,
        &event);

    if( res == RES_OK ) {
        broker_publish(&event);
    }
}

static void llm_stop_event_publish( void ) {
    event_t event = STRUCT_INIT_ALL_ZEROS;
    result_t res = event_create(
        COMPONENT_CORE_DISP, COMPONENT_LLM,
        EVENT_LLM_STOP, 
        NULL, 0,
        &event);

    if( res == RES_OK ) {
        broker_publish(&event);
    }
}

static void action_based_on_button( core_context_t * context, 
        button_gpio_t button ) {
    switch( button ) {
        case BUTTON_UP_GPIO: {
            if( context->state == CORE_STATE_WAIT_FOR_START ) {
                if( context->ans.last_answer_file ) {
                    scroll_forward_last_answer(context);
                }
            }

            break;
        }

        case BUTTON_OK_GPIO: {
            switch( context->state ) {
                case CORE_STATE_WAIT_FOR_START: {
                    answer_context_reinit(&context->ans);
                    rec_request_event_publish();
                    break;
                }

                case CORE_STATE_REC_PROCESSING: {
                    if( HAS_TIME_PASSED(context->last_time_pressed_ok_us, 
                            1 * US_PER_SEC) ) {
                        rec_stop_event_publish();
                    }
                    break;
                }

                case CORE_STATE_STT_PROCESSING: {
                    if( HAS_TIME_PASSED(context->last_time_pressed_ok_us, 
                            1 * US_PER_SEC) ) {
                        stt_stop_event_publish();
                    }
                    break;
                }

                case CORE_STATE_LLM_PROCESSING: {
                    if( HAS_TIME_PASSED(context->last_time_pressed_ok_us, 
                            1 * US_PER_SEC) ) {
                        llm_stop_event_publish();
                    }
                    break;
                }

                default:
                    break;
            }

            update_last_time_pressed(&context->last_time_pressed_ok_us);

            break;
        }

        case BUTTON_DOWN_GPIO: {
            if( context->state == CORE_STATE_WAIT_FOR_START ) {
                if( context->ans.last_answer_file ) {
                    scroll_backward_last_answer(context);
                }
            }

            break;
        }

        default: 
            break;
    }
}

// === DISPLAY ANSWER ===

static void answer_context_reinit( answer_context_t * context ) {
    if( context->last_answer_file ) {
        fclose(context->last_answer_file);
    }
    context->page_pos = 0;
    memset(context->page_pos_history, 0, sizeof(context->page_pos_history));
    context->page_pos_history_idx = -1;
}

static result_t answer_read_char( answer_context_t * context, char * ch ) {
    fseek(context->last_answer_file, context->page_pos, SEEK_SET);  
    int result = fgetc(context->last_answer_file); 
    if( result != EOF ) {
        *ch = (char)result;
        context->page_pos = ftell(context->last_answer_file); 
        return RES_OK; 
    }
    return RES_ERR_GENERIC;
}

static result_t answer_read_word( answer_context_t * context, char * word, 
        int max_len ) {
    int i = 0;
    char ch;

    while( i < max_len - 1 ) {
        if( answer_read_char(context, &ch) != RES_OK ) {
            context->page_pos = EOF_POS;
            break;  
        }

        if( ch == ' ' || ch == '\n' ) {
            if( i > 0 ) {  
                word[i++] = ch;
                word[i] = '\0';
                return RES_OK;
            }
            word[i++] = ch;
            word[i] = '\0';
            return RES_OK;
        }

        word[i++] = ch; 
    }

    if( i > 0 ) {
        word[i] = '\0';
        return RES_OK;
    }

    return RES_ERR_GENERIC;
}

static result_t answer_open_file( answer_context_t * context, char * path ) {
    context->last_answer_file = fopen(path, "r");
    RETURN_IF_NULL(context->last_answer_file);
    return RES_OK;
}

static void scroll_forward_last_answer( core_context_t * context ) {
    if( context->ans.page_pos == EOF_POS ) {
        context->ans.page_pos = 0;
        context->ans.page_pos_history_idx = 0;
    }
    
    if( context->ans.page_pos_history_idx < (int)(NELEMS(context->ans.page_pos_history) - 1) ) {
        context->ans.page_pos_history_idx++;
        context->ans.page_pos_history[context->ans.page_pos_history_idx] = context->ans.page_pos;
    }

    display_menu_clear(&context->menu);

    char word[32];
    while( !is_display_menu_almost_full(&context->menu) ) {
        if( answer_read_word(&context->ans, word, NELEMS(word)) != RES_OK ) {
            break;
        }

        display_menu_append_text(&context->menu, word, COLOR_FULL_OUTPUT);
    }
}

static void scroll_backward_last_answer( core_context_t * context ) {
    if( context->ans.page_pos_history_idx > 0 ) {
        context->ans.page_pos_history_idx--;
        context->ans.page_pos = context->ans.page_pos_history[context->ans.page_pos_history_idx];

        display_menu_clear(&context->menu);

        char word[32];
        while( !is_display_menu_almost_full(&context->menu) ) {
            if( answer_read_word(&context->ans, word, NELEMS(word)) != RES_OK ) {
                break;
            } 
            
            display_menu_append_text(&context->menu, word, COLOR_FULL_OUTPUT);
        }
    }
}

/********************
 * GLOBAL FUNCTIONS *
 ********************/

void * core_thread( void * arg UNUSED_PARAM ) {
    core_context_t context = {
        .state = CORE_STATE_WAIT_FOR_START,
        .menu = DEFAULT_DISPLAY_MENU,

        .last_time_pressed_ok_us = 0
    };
    answer_context_reinit(&context.ans);

    show_welcome_text(&context.menu);

    while(1) {
        event_t e = STRUCT_INIT_ALL_ZEROS;
        if( broker_pop(COMPONENT_CORE_DISP, &e) == RES_OK ) {
            switch( e.type ) {
                case EVENT_BUT_PRESSED: {
                    button_gpio_t gpio;
                    if( e.data_size != sizeof(gpio) ) {
                        display_menu_append_text(&context.menu, 
                            "Failed GPIO data in event.\n", COLOR_STATUS);
                        continue;
                    }
                    memcpy(&gpio, e.data, e.data_size);

                    action_based_on_button(&context, gpio);

                    break;
                }

                case EVENT_PIPELINE_DONE: {
                    context.state = CORE_STATE_WAIT_FOR_START;

                    display_menu_append_text(&context.menu, "Pipeline done.\n", COLOR_STATUS);

                    char prompt_with_answer_path[EVENT_MAX_DATA_SIZE];
                    memcpy(prompt_with_answer_path, e.data, e.data_size);
                    
                    if( answer_open_file(&context.ans, prompt_with_answer_path) != RES_OK ) {
                        display_menu_append_text(&context.menu, 
                            "Can't open file with prompt and answer.\n", COLOR_STATUS);
                    } else {
                        show_final_text(&context.menu);
                    }

                    break;
                }

                case EVENT_REC_STATUS: {
                    context.state = CORE_STATE_REC_PROCESSING;

                    char msg[EVENT_MAX_DATA_SIZE];
                    snprintf(msg, sizeof(msg), "%.*s", (int)e.data_size, e.data);
                    display_menu_update_line(&context.menu, msg, COLOR_STATUS);

                    break;
                }

                case EVENT_STT_STATUS: {
                    context.state = CORE_STATE_STT_PROCESSING;

                    char msg[EVENT_MAX_DATA_SIZE];
                    snprintf(msg, sizeof(msg), "%.*s", (int)e.data_size, e.data);
                    display_menu_update_line(&context.menu, msg, COLOR_STATUS);

                    break;
                }

                case EVENT_LLM_STATUS: {
                    context.state = CORE_STATE_LLM_PROCESSING;

                    char msg[EVENT_MAX_DATA_SIZE];
                    snprintf(msg, sizeof(msg), "%.*s", (int)e.data_size, e.data);
                    display_menu_append_text(&context.menu, msg, COLOR_PARTIAL_ANSWER);

                    break;
                }

                default:
                    break;
            }
        }

        usleep(30000);
    }

    return NULL;
}

result_t core_init( void ) {
    return RES_OK;
}
