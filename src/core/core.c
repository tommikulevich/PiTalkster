#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include "utils.h"

#include "core.h"
#include "event_broker.h"
#include "controls.h"
#include "controls_gpio.h"


void * core_thread( void * arg UNUSED_PARAM ) {
    while(1) {
        event_t e = STRUCT_INIT_ALL_ZEROS;
        if( broker_pop(COMPONENT_CORE_DISP, &e) == RES_OK ) {
            switch( e.type ) {
                case EVENT_BUT_PRESSED: {
                    button_gpio_t gpio;
                    if( e.data_size != sizeof(gpio) ) {
                        ERROR("Failed GPIO data in event.");
                        continue;
                    }
    
                    memcpy(&gpio, e.data, e.data_size);

                    switch( gpio ) {
                        case BUTTON_UP_GPIO: {
                            event_t event = STRUCT_INIT_ALL_ZEROS;
                            result_t res = event_create(
                                COMPONENT_CORE_DISP, COMPONENT_AUDIO_INPUT,
                                EVENT_REC_REQUEST, 
                                NULL, 0,
                                &event);

                            if( res == RES_OK ) {
                                broker_publish(&event);
                            }
                            
                            break;
                        }

                        case BUTTON_OK_GPIO: {
                            break;
                        }

                        case BUTTON_DOWN_GPIO: {
                            // TODO: cancel another operations
                            event_t event = STRUCT_INIT_ALL_ZEROS;
                            result_t res = event_create(
                                COMPONENT_CORE_DISP, COMPONENT_AUDIO_INPUT,
                                EVENT_REC_CANCEL, 
                                NULL, 0,
                                &event);

                            if( res == RES_OK ) {
                                broker_publish(&event);
                            }
                        }

                        default: 
                            break;
                    }

                    break;
                }

                case EVENT_PIPELINE_DONE: {
                    INFO("Yeah, all things done!");
                    break;
                }

                case EVENT_REC_ERROR: {
                    INFO("Oh no, recording error!");
                    break;
                }

                case EVENT_STT_ERROR: {
                    INFO("Oh no, STT error!");
                    break;
                }

                case EVENT_LLM_ERROR: {
                    INFO("Oh no, LLM error!");
                    break;
                }

                case EVENT_DISPLAY_UPDATE: {
                    INFO("Display need update...");
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
