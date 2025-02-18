#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include "utils.h"

#include "core.h"
#include "event_broker.h"


void * core_thread( void * arg UNUSED_PARAM ) {
    while(1) {
        event_t e = STRUCT_INIT_ALL_ZEROS;
        if( broker_pop(COMPONENT_CORE, &e) == RES_OK ) {
            switch( e.event_type ) {
                case EVENT_BUT_PRESSED: {
                    INFO("I have it, GPIO!");
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
