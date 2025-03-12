/**
 *******************************************************************************
 * @file    audio_input.c
 * @brief   Audio input source file.
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
#include <errno.h>

#include "utils.h"

#include "audio_input.h"
#include "audio_input_rec_ops.h"
#include "event_broker.h"

/******************************
 * PRIVATE MACROS AND DEFINES *
 ******************************/

#define DEFAULT_REC_FOLDER      "data"
#define DEFAULT_MAX_REC_DUR_S   60      // TODO: make it configurable
#define MAX_FILEPATH_SIZE       512

/********************
 * PRIVATE TYPEDEFS *
 ********************/

typedef enum {
    REC_STATUS_NOT_STARTED = 0,
    REC_STATUS_IN_PROGRESS,
    REC_STATUS_FINISHED_OK,
    REC_STATUS_FINISHED_ERROR,
} rec_status_t;

typedef struct {
    char wav_filepath[MAX_FILEPATH_SIZE];
    int duration_s;

    volatile int * rec_stop_flag;
    volatile int * rec_progress;

    rec_status_t status;
} rec_context_t;

/********************
 * STATIC VARIABLES *
 ********************/

static volatile int rec_stop_flag = 0;
static volatile int rec_progress = 0;

/********************
 * STATIC FUNCTIONS *
 ********************/

static result_t create_wav_filepath_with_date( char * filepath, 
        size_t filepath_size ) {
    RETURN_IF_NULL(filepath);
    
    time_t now = time(NULL);
    struct tm * t = localtime(&now);

    char folderpath[MAX_FILEPATH_SIZE / 2];
    RETURN_ERROR_IF( strftime(folderpath, sizeof(folderpath), 
        DEFAULT_REC_FOLDER"/%Y-%m-%d_%H-%M-%S", t) == 0, 
        RES_ERR_GENERIC );
    RETURN_ERROR_IF( mkdir(folderpath, 0755) != 0 && errno != EEXIST, 
        RES_ERR_GENERIC);

    char filename[MAX_FILEPATH_SIZE / 2];
    RETURN_ERROR_IF( strftime(filename, sizeof(filename), 
        "rec_%Y-%m-%d_%H-%M-%S.wav", t) == 0,
        RES_ERR_GENERIC );
    RETURN_ERROR_IF( snprintf(filepath, filepath_size, "%s/%s", 
        folderpath, filename) < 0,
        RES_ERR_GENERIC );

    return RES_OK;
}

static void * record_thread( void * arg ) {
    rec_context_t * params = (rec_context_t *)arg;

    params->status = REC_STATUS_IN_PROGRESS;
    result_t res = record_audio_to_wav(params->wav_filepath, params->duration_s, 
        params->rec_stop_flag, params->rec_progress);
    params->status = (res == RES_OK) ? REC_STATUS_FINISHED_OK : 
                                       REC_STATUS_FINISHED_ERROR;

    return NULL;
}

static void stt_request_event_publish( char * wav_filepath, 
        size_t wav_filepath_size ) {    
    event_t event = STRUCT_INIT_ALL_ZEROS;
    result_t res = event_create(
        COMPONENT_AUDIO_INPUT, COMPONENT_STT,
        EVENT_STT_REQUEST, 
        wav_filepath, wav_filepath_size,
        &event);

    if( res == RES_OK ) {
        broker_publish(&event);
    }
}

static void rec_status_event_publish( const char * status_msg, 
        size_t status_msg_size ) {    
    event_t event = STRUCT_INIT_ALL_ZEROS;
    result_t res = event_create(
        COMPONENT_AUDIO_INPUT, COMPONENT_CORE_DISP,
        EVENT_REC_STATUS, 
        status_msg, status_msg_size,
        &event);

    if( res == RES_OK ) {
        broker_publish(&event);
    }
}

static void rec_context_clear( rec_context_t * context ) {
    memset(context->wav_filepath, 0, sizeof(context->wav_filepath));
    
    rec_stop_flag = 0;
    rec_progress = 0;

    context->status = REC_STATUS_NOT_STARTED;
}

/********************
 * GLOBAL FUNCTIONS *
 ********************/

void * audio_input_thread( void * arg UNUSED_PARAM ) {
    pthread_t rec_thread;
    rec_context_t context = {
        .duration_s = DEFAULT_MAX_REC_DUR_S,

        .rec_stop_flag = &rec_stop_flag,
        .rec_progress = &rec_progress,

        .status = REC_STATUS_NOT_STARTED
    };

    while(1) {
        // Action based on actual operation status
        switch( context.status ) {
            case REC_STATUS_NOT_STARTED:
                break;

            case REC_STATUS_IN_PROGRESS: {
                DO_WITH_INTERVAL_MS(500, {
                    char status_msg[32];
                    snprintf(status_msg, sizeof(status_msg), 
                        "Recording progress: %d/%ds", *context.rec_progress,
                        context.duration_s);
                    rec_status_event_publish(status_msg, 
                        strlen(status_msg));
                });
                break;
            }

            case REC_STATUS_FINISHED_OK: {
                const char * status_msg = "Recording finished.\n";
                rec_status_event_publish(status_msg, 
                    strlen(status_msg));
                stt_request_event_publish(context.wav_filepath, 
                    sizeof(context.wav_filepath));
                rec_context_clear(&context);
                break;
            }

            case REC_STATUS_FINISHED_ERROR: {
                const char * error_msg = "\nError: Recording failed.\n";
                rec_status_event_publish(error_msg, 
                    strlen(error_msg));
                rec_context_clear(&context);
                break;
            }

            default:
                break;
        }

        // Action based on incomming event
        event_t e = STRUCT_INIT_ALL_ZEROS;
        if( broker_pop(COMPONENT_AUDIO_INPUT, &e) == RES_OK ) {
            switch( e.type ) {
                case EVENT_REC_REQUEST: {
                    if( context.status != REC_STATUS_NOT_STARTED ) {
                        const char * status_msg = 
                            "Ignoring recording request (already started).\n";
                        rec_status_event_publish(status_msg, 
                            strlen(status_msg));
                        continue;
                    }

                    result_t res = create_wav_filepath_with_date(context.wav_filepath, 
                        sizeof(context.wav_filepath));
                    if( res != RES_OK ) {
                        const char * error_msg = 
                            "Error: Failed to create WAV path.\n";
                        rec_status_event_publish(error_msg, 
                            strlen(error_msg));
                        continue;
                    }

                    const char * msg = "Recording start.\n";
                    rec_status_event_publish(msg, strlen(msg));

                    rec_stop_flag = 0;
                    pthread_create(&rec_thread, NULL, record_thread, &context);

                    break;
                }

                case EVENT_REC_STOP: {
                    if( context.status != REC_STATUS_IN_PROGRESS ) {
                        const char * status_msg = 
                            "Ignoring stopping recording request (not started).\n";
                        rec_status_event_publish(status_msg, 
                            strlen(status_msg));
                        continue;
                    }

                    rec_stop_flag = 1;
                    pthread_join(rec_thread, NULL);
                    
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

result_t audio_input_init( void ) {
    return RES_OK;
}
