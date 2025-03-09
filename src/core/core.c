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

/********************
 * STATIC FUNCTIONS *
 ********************/

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

/********************
 * GLOBAL FUNCTIONS *
 ********************/

void * core_thread( void * arg UNUSED_PARAM ) {
    display_menu_t menu = STRUCT_INIT_ALL_ZEROS;

    while(1) {
        event_t e = STRUCT_INIT_ALL_ZEROS;
        if( broker_pop(COMPONENT_CORE_DISP, &e) == RES_OK ) {
            switch( e.type ) {
                case EVENT_BUT_PRESSED: {
                    button_gpio_t gpio;
                    if( e.data_size != sizeof(gpio) ) {
                        display_menu_append_text(&menu, 
                            "Failed GPIO data in event.\n", COLOR_STATUS);
                        continue;
                    }
    
                    memcpy(&gpio, e.data, e.data_size);

                    switch( gpio ) {
                        case BUTTON_UP_GPIO: {
                            // TODO: start another operations depending on state
                            rec_request_event_publish();
                            break;
                        }

                        case BUTTON_OK_GPIO: {
                            break;
                        }

                        case BUTTON_DOWN_GPIO: {
                            // TODO: stop another operations depending on state
                            rec_stop_event_publish();
                        }

                        default: 
                            break;
                    }

                    break;
                }

                case EVENT_PIPELINE_DONE: {
                    display_menu_append_text(&menu, "Pipeline done.\n", COLOR_STATUS);
                    break;
                }

                case EVENT_REC_STATUS: {
                    char msg[EVENT_MAX_DATA_SIZE];
                    snprintf(msg, sizeof(msg), "%.*s", (int)e.data_size, e.data);
                    display_menu_update_line(&menu, msg, COLOR_STATUS);
                    break;
                }

                case EVENT_STT_STATUS: {
                    char msg[EVENT_MAX_DATA_SIZE];
                    snprintf(msg, sizeof(msg), "%.*s", (int)e.data_size, e.data);
                    display_menu_update_line(&menu, msg, COLOR_STATUS);
                    break;
                }

                case EVENT_LLM_STATUS: {
                    char msg[EVENT_MAX_DATA_SIZE];
                    snprintf(msg, sizeof(msg), "%.*s", (int)e.data_size, e.data);
                    display_menu_append_text(&menu, msg, COLOR_ANSWER);
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
