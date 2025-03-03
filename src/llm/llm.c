#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>

#include "utils.h"

#include "llm.h"
#include "ollama_api_ops.h"
#include "event_broker.h"

#define MAX_FILEPATH_SIZE       512

typedef enum {
    LLM_STATUS_NOT_STARTED = 0,
    LLM_STATUS_IN_PROGRESS,
    LLM_STATUS_FINISHED_OK,
    LLM_STATUS_FINISHED_ERROR,
} llm_status_t;

typedef struct {
    char prompt_filepath[MAX_FILEPATH_SIZE];
    char answer_filepath[MAX_FILEPATH_SIZE];
    volatile int * llm_stop_flag;

    llm_status_t status;
} llm_context_t;

volatile int llm_stop_flag = 0;

static void create_answer_filepath_from_prompt_filepath( char * answer_filepath, 
        size_t answer_filepath_size, char * prompt_filepath, size_t prompt_filepath_size ) {
    ASSERT_NOT_NULL(answer_filepath);
    ASSERT(answer_filepath_size != 0);
    ASSERT_NOT_NULL(prompt_filepath);
    ASSERT(strlen(prompt_filepath) != 0);

    char temp_filepath[prompt_filepath_size];
    strncpy(temp_filepath, prompt_filepath, prompt_filepath_size);
    temp_filepath[sizeof(temp_filepath) - 1] = '\0';

    char * dot = strrchr(temp_filepath, '.');
    if( dot == NULL ) {
        ASSERT(0);
        return;
    }

    if( (strlen(temp_filepath) + 1) > answer_filepath_size ) {
        ASSERT(0);
        return;
    }

    memcpy(dot, ".out", 4);
    dot[4] = '\0'; 

    strncpy(answer_filepath, temp_filepath, answer_filepath_size);
    answer_filepath[answer_filepath_size - 1] = '\0'; 
}

static void * llm_operation_thread( void * arg ) {
    llm_context_t *params = (llm_context_t *)arg;

    result_t res = ollama_ask_deepseek_model(params->answer_filepath, params->prompt_filepath, 
        params->llm_stop_flag);
    params->status = (res == RES_OK) ? LLM_STATUS_FINISHED_OK : 
                                       LLM_STATUS_FINISHED_ERROR;

    return NULL;
}

void * llm_thread( void * arg UNUSED_PARAM ) {
    pthread_t llm_op_thread;
    llm_context_t context = {
        .llm_stop_flag = &llm_stop_flag,

        .status = LLM_STATUS_NOT_STARTED
    };

    while(1) {
        if( context.status == LLM_STATUS_FINISHED_OK 
                || context.status == LLM_STATUS_FINISHED_ERROR ) {
            INFO("LLM finished.");

            event_t event = STRUCT_INIT_ALL_ZEROS;
            if( context.status == LLM_STATUS_FINISHED_OK ) {                        
                result_t res = event_create(
                    COMPONENT_LLM, COMPONENT_CORE_DISP,
                    EVENT_PIPELINE_DONE, 
                    context.answer_filepath, sizeof(context.answer_filepath),
                    &event);

                if( res == RES_OK ) {
                    broker_publish(&event);
                }
            } else {
                result_t res = event_create(
                    COMPONENT_LLM, COMPONENT_CORE_DISP,
                    EVENT_LLM_ERROR, 
                    NULL, 0,
                    &event);

                if( res == RES_OK ) {
                    broker_publish(&event);
                }
            }

            memset(context.answer_filepath, 0, sizeof(context.answer_filepath));
            llm_stop_flag = 0;
            context.status = LLM_STATUS_NOT_STARTED;
        }

        event_t e = STRUCT_INIT_ALL_ZEROS;
        if( broker_pop(COMPONENT_LLM, &e) == RES_OK ) {
            switch( e.type ) {
                case EVENT_LLM_REQUEST: {
                    if( context.status != LLM_STATUS_NOT_STARTED ) {
                        INFO("Ignoring LLM request (already started).");
                        continue;
                    }

                    if( e.data_size > sizeof(context.prompt_filepath) ) {
                        ERROR("Failed TXT path in event.");
                        continue;
                    }

                    memcpy(context.prompt_filepath, e.data, e.data_size);

                    INFO("LLM requested.");

                    create_answer_filepath_from_prompt_filepath(context.answer_filepath, 
                        sizeof(context.answer_filepath), context.prompt_filepath,
                        sizeof(context.prompt_filepath));
                    llm_stop_flag = 0;
                    context.status = LLM_STATUS_IN_PROGRESS;

                    pthread_create(&llm_op_thread, NULL, llm_operation_thread, &context);
                    break;
                }

                case EVENT_LLM_CANCEL: {
                    if( context.status != LLM_STATUS_IN_PROGRESS ) {
                        INFO("Ignoring canceling LLM request (not started).");
                        continue;
                    }

                    INFO("LLM canceled.");

                    llm_stop_flag = 1;
                    pthread_join(llm_op_thread, NULL);
                    
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

result_t llm_init( void ) {
    // TODO: add check if ollama service is alive 
    return RES_OK;
}
