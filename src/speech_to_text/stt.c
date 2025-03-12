/**
 *******************************************************************************
 * @file    stt.c
 * @brief   Speech-to-text source file. 
 *          Includes thread with event-driven part.
 *******************************************************************************
 */

/************
 * INCLUDES *
 ************/

#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>

#include "utils.h"

#include "stt.h"
#include "stt_ops.h"
#include "event_broker.h"

/******************************
 * PRIVATE MACROS AND DEFINES *
 ******************************/

#define MAX_FILEPATH_SIZE       512

/********************
 * PRIVATE TYPEDEFS *
 ********************/

typedef enum {
    STT_STATUS_NOT_STARTED = 0,
    STT_STATUS_IN_PROGRESS,
    STT_STATUS_FINISHED_OK,
    STT_STATUS_FINISHED_ERROR,
} stt_status_t;

typedef struct {
    char wav_filepath[MAX_FILEPATH_SIZE];
    char txt_filepath[MAX_FILEPATH_SIZE];

    volatile int * stt_stop_flag;
    volatile int * stt_progress;

    stt_status_t status;
} stt_context_t;

/********************
 * STATIC VARIABLES *
 ********************/

static volatile int stt_stop_flag = 0;
static volatile int stt_progress = 0;

/********************
 * STATIC FUNCTIONS *
 ********************/

static result_t create_txt_filepath_from_wav_filepath( 
        char * txt_filepath, size_t txt_filepath_size, 
        char * wav_filepath, size_t wav_filepath_size ) {
    RETURN_IF_NULL(txt_filepath);
    RETURN_ERROR_IF( txt_filepath_size == 0, RES_ERR_WRONG_ARGS );
    RETURN_IF_NULL(wav_filepath);
    RETURN_ERROR_IF( strlen(wav_filepath) == 0, RES_ERR_WRONG_ARGS );

    char temp_filepath[wav_filepath_size];
    strncpy(temp_filepath, wav_filepath, wav_filepath_size);
    temp_filepath[sizeof(temp_filepath) - 1] = '\0';

    char * dot = strrchr(temp_filepath, '.');
    RETURN_ERROR_IF( !dot, RES_ERR_GENERIC );

    RETURN_ERROR_IF( (strlen(temp_filepath) + 1) > txt_filepath_size, 
        RES_ERR_GENERIC );

    memcpy(dot, ".txt", 4);
    dot[4] = '\0'; 

    strncpy(txt_filepath, temp_filepath, txt_filepath_size);
    txt_filepath[txt_filepath_size - 1] = '\0'; 

    return RES_OK;
}

static void * stt_operation_thread( void * arg ) {
    stt_context_t * params = (stt_context_t *)arg;

    params->status = STT_STATUS_IN_PROGRESS;
    result_t res = perform_speech_to_text(params->txt_filepath, params->wav_filepath, 
        params->stt_stop_flag, params->stt_progress);
    params->status = (res == RES_OK) ? STT_STATUS_FINISHED_OK : 
                                       STT_STATUS_FINISHED_ERROR;

    return NULL;
}

static void llm_request_event_publish( char * txt_filepath, 
        size_t txt_filepath_size ) {    
    event_t event = STRUCT_INIT_ALL_ZEROS;
    result_t res = event_create(
        COMPONENT_STT, COMPONENT_LLM,
        EVENT_LLM_REQUEST, 
        txt_filepath, txt_filepath_size,
        &event);

    if( res == RES_OK ) {
        broker_publish(&event);
    }
}

static void stt_status_event_publish( const char * status_msg, 
        size_t status_msg_size ) {    
    event_t event = STRUCT_INIT_ALL_ZEROS;
    result_t res = event_create(
        COMPONENT_STT, COMPONENT_CORE_DISP,
        EVENT_STT_STATUS, 
        status_msg, status_msg_size,
        &event);

    if( res == RES_OK ) {
        broker_publish(&event);
    }
}

static void pipeline_failed_event_publish( void ) {    
    event_t event = STRUCT_INIT_ALL_ZEROS;
    result_t res = event_create(
        COMPONENT_STT, COMPONENT_CORE_DISP,
        EVENT_PIPELINE_DONE, 
        NULL, 0,
        &event);

    if( res == RES_OK ) {
        broker_publish(&event);
    }
}

static void stt_context_clear( stt_context_t * context ) {
    memset(context->wav_filepath, 0, sizeof(context->wav_filepath));
    memset(context->txt_filepath, 0, sizeof(context->txt_filepath));
    
    stt_stop_flag = 0;
    stt_progress = 0;

    context->status = STT_STATUS_NOT_STARTED;
}

/********************
 * GLOBAL FUNCTIONS *
 ********************/

void * stt_thread( void * arg UNUSED_PARAM ) {
    pthread_t stt_op_thread;
    stt_context_t context = {
        .stt_stop_flag = &stt_stop_flag,
        .stt_progress = &stt_progress,

        .status = STT_STATUS_NOT_STARTED
    };

    while(1) {
        // Action based on actual operation status
        switch( context.status ) {
            case STT_STATUS_NOT_STARTED:
                break;

            case STT_STATUS_IN_PROGRESS: {
                DO_WITH_INTERVAL_MS(500, {
                    char status_msg[32];
                    snprintf(status_msg, sizeof(status_msg), 
                        "STT progress: %d/100%%", *context.stt_progress);
                    stt_status_event_publish(status_msg, 
                        strlen(status_msg));
                });
                break;
            }

            case STT_STATUS_FINISHED_OK: {
                const char * status_msg = "STT finished.\n";
                stt_status_event_publish(status_msg, 
                    strlen(status_msg));
                llm_request_event_publish(context.txt_filepath, 
                    sizeof(context.txt_filepath));
                stt_context_clear(&context);
                break;
            }

            case STT_STATUS_FINISHED_ERROR: {
                const char * error_msg = "\nError: STT failed.\n";
                stt_status_event_publish(error_msg, 
                    strlen(error_msg));
                pipeline_failed_event_publish();
                stt_context_clear(&context);
                break;
            }

            default:
                break;
        }

        // Action based on incomming event
        event_t e = STRUCT_INIT_ALL_ZEROS;
        if( broker_pop(COMPONENT_STT, &e) == RES_OK ) {
            switch( e.type ) {
                case EVENT_STT_REQUEST: {
                    if( context.status != STT_STATUS_NOT_STARTED ) {
                        const char * status_msg = 
                            "Ignoring STT request (already started).\n";
                        stt_status_event_publish(status_msg, 
                            strlen(status_msg));
                        continue;
                    }

                    if( e.data_size > sizeof(context.wav_filepath) ) {
                        const char * error_msg = 
                            "Error: Failed WAV path in event.\n";
                        stt_status_event_publish(error_msg, 
                            strlen(error_msg));
                        continue;
                    }

                    memcpy(context.wav_filepath, e.data, e.data_size);

                    result_t res = create_txt_filepath_from_wav_filepath(
                        context.txt_filepath, sizeof(context.txt_filepath), 
                        context.wav_filepath, sizeof(context.wav_filepath));
                    if( res != RES_OK ) {
                        const char * error_msg = 
                            "Error: Failed to create TXT path.\n";
                        stt_status_event_publish(error_msg, 
                            strlen(error_msg));
                        continue;
                    }

                    const char * msg = "STT start.\n";
                    stt_status_event_publish(msg, strlen(msg));

                    stt_stop_flag = 0;
                    pthread_create(&stt_op_thread, NULL, stt_operation_thread, &context);
                    
                    break;
                }

                case EVENT_STT_STOP: {
                    if( context.status != STT_STATUS_IN_PROGRESS ) {
                        const char * status_msg = 
                            "Ignoring stopping STT request (not started).\n";
                        stt_status_event_publish(status_msg, 
                            strlen(status_msg));
                        continue;
                    }

                    stt_stop_flag = 1;
                    pthread_join(stt_op_thread, NULL);
                    
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

result_t stt_init( void ) {
    return RES_OK;
}
