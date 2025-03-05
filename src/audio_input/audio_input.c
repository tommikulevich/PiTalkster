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

#define DEFAULT_REC_FOLDER      "data"
#define DEFAULT_MAX_REC_DUR_S   60      // TODO: make it configurable
#define MAX_FILEPATH_SIZE       512

typedef enum {
    REC_STATUS_NOT_STARTED = 0,
    REC_STATUS_IN_PROGRESS,
    REC_STATUS_FINISHED_OK,
    REC_STATUS_FINISHED_ERROR,
} rec_status_t;

typedef struct {
    char filepath[MAX_FILEPATH_SIZE];
    int duration_s;
    volatile int * rec_stop_flag;

    rec_status_t status;
} record_context_t;

volatile int rec_stop_flag = 0;

static result_t create_wav_filepath_with_date( char * filepath OUTPUT, 
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
    record_context_t * params = (record_context_t *)arg;

    result_t res = record_audio(params->filepath, params->duration_s, 
        params->rec_stop_flag);
    params->status = (res == RES_OK) ? REC_STATUS_FINISHED_OK : 
                                       REC_STATUS_FINISHED_ERROR;

    return NULL;
}

void * audio_input_thread( void * arg UNUSED_PARAM ) {
    pthread_t rec_thread;
    record_context_t context = {
        .duration_s = DEFAULT_MAX_REC_DUR_S,
        .rec_stop_flag = &rec_stop_flag,

        .status = REC_STATUS_NOT_STARTED
    };

    while(1) {
        if( context.status == REC_STATUS_FINISHED_OK
                || context.status == REC_STATUS_FINISHED_ERROR ) {
            INFO("Recording finished.");
            
            event_t event = STRUCT_INIT_ALL_ZEROS;
            if( context.status == REC_STATUS_FINISHED_OK ) {                        
                result_t res = event_create(
                    COMPONENT_AUDIO_INPUT, COMPONENT_STT,
                    EVENT_STT_REQUEST, 
                    context.filepath, sizeof(context.filepath),
                    &event);

                if( res == RES_OK ) {
                    broker_publish(&event);
                }
            } else {
                result_t res = event_create(
                    COMPONENT_AUDIO_INPUT, COMPONENT_CORE_DISP,
                    EVENT_REC_ERROR, 
                    NULL, 0,
                    &event);

                if( res == RES_OK ) {
                    broker_publish(&event);
                }
            }

            memset(context.filepath, 0, sizeof(context.filepath));
            rec_stop_flag = 0;
            context.status = REC_STATUS_NOT_STARTED;
        }

        event_t e = STRUCT_INIT_ALL_ZEROS;
        if( broker_pop(COMPONENT_AUDIO_INPUT, &e) == RES_OK ) {
            switch( e.type ) {
                case EVENT_REC_REQUEST: {
                    if( context.status != REC_STATUS_NOT_STARTED ) {
                        INFO("Ignoring recording request (already started).");
                        continue;
                    }

                    result_t res = create_wav_filepath_with_date(context.filepath, 
                        sizeof(context.filepath));
                    if( res != RES_OK ) {
                        ERROR("Failed to create WAV path.");
                        continue;
                    }

                    INFO("Recording requested.");

                    rec_stop_flag = 0;
                    context.status = REC_STATUS_IN_PROGRESS;

                    pthread_create(&rec_thread, NULL, record_thread, &context);
                    break;
                }

                case EVENT_REC_CANCEL: {
                    if( context.status != REC_STATUS_IN_PROGRESS ) {
                        INFO("Ignoring canceling request (not started).");
                        continue;
                    }

                    INFO("Recording canceled.");

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
