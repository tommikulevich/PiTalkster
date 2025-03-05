#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>

#include "utils.h"

#include "stt.h"
#include "stt_ops.h"
#include "event_broker.h"

#define MAX_FILEPATH_SIZE       512

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

    stt_status_t status;
} stt_context_t;

volatile int stt_stop_flag = 0;

static result_t create_txt_filepath_from_wav_filepath( char * txt_filepath, 
        size_t txt_filepath_size, char * wav_filepath, size_t wav_filepath_size ) {
    RETURN_IF_NULL(txt_filepath);
    RETURN_ERROR_IF( txt_filepath_size == 0, 
        RES_ERR_GENERIC );
    RETURN_IF_NULL(wav_filepath);
    RETURN_ERROR_IF( strlen(wav_filepath) == 0,
        RES_ERR_GENERIC );

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

    result_t res = perform_speech_to_text(params->txt_filepath, params->wav_filepath, 
        params->stt_stop_flag);
    params->status = (res == RES_OK) ? STT_STATUS_FINISHED_OK : 
                                       STT_STATUS_FINISHED_ERROR;

    return NULL;
}

void * stt_thread( void * arg UNUSED_PARAM ) {
    pthread_t stt_op_thread;
    stt_context_t context = {
        .stt_stop_flag = &stt_stop_flag,

        .status = STT_STATUS_NOT_STARTED
    };

    while(1) {
        if( context.status == STT_STATUS_FINISHED_OK 
                || context.status == STT_STATUS_FINISHED_ERROR ) {
            INFO("STT finished.");

            event_t event = STRUCT_INIT_ALL_ZEROS;
            if( context.status == STT_STATUS_FINISHED_OK ) {                        
                result_t res = event_create(
                    COMPONENT_STT, COMPONENT_LLM,
                    EVENT_LLM_REQUEST, 
                    context.txt_filepath, sizeof(context.txt_filepath),
                    &event);

                if( res == RES_OK ) {
                    broker_publish(&event);
                }
            } else {
                result_t res = event_create(
                    COMPONENT_STT, COMPONENT_CORE_DISP,
                    EVENT_STT_ERROR, 
                    NULL, 0,
                    &event);

                if( res == RES_OK ) {
                    broker_publish(&event);
                }
            }

            memset(context.txt_filepath, 0, sizeof(context.txt_filepath));
            stt_stop_flag = 0;
            context.status = STT_STATUS_NOT_STARTED;
        }

        event_t e = STRUCT_INIT_ALL_ZEROS;
        if( broker_pop(COMPONENT_STT, &e) == RES_OK ) {
            switch( e.type ) {
                case EVENT_STT_REQUEST: {
                    if( context.status != STT_STATUS_NOT_STARTED ) {
                        INFO("Ignoring STT request (already started).");
                        continue;
                    }

                    if( e.data_size > sizeof(context.wav_filepath) ) {
                        ERROR("Failed WAV path in event.");
                        continue;
                    }

                    memcpy(context.wav_filepath, e.data, e.data_size);

                    result_t res = create_txt_filepath_from_wav_filepath(context.txt_filepath, 
                        sizeof(context.txt_filepath), context.wav_filepath,
                        sizeof(context.wav_filepath));
                    if( res != RES_OK ) {
                        ERROR("Failed to create TXT path.");
                        continue;
                    }

                    INFO("STT requested.");

                    stt_stop_flag = 0;
                    context.status = STT_STATUS_IN_PROGRESS;

                    pthread_create(&stt_op_thread, NULL, stt_operation_thread, &context);
                    break;
                }

                case EVENT_STT_CANCEL: {
                    if( context.status != STT_STATUS_IN_PROGRESS ) {
                        INFO("Ignoring canceling STT request (not started).");
                        continue;
                    }

                    INFO("STT canceled.");

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
