/**
 ********************************************************************************
 * @file    llm.c
 * @brief   Large language model interaction source file.
 *          Includes thread with event-driven part.
 ******************************************************************************
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

#include "llm.h"
#include "ollama_api_ops.h"
#include "event_broker.h"

/******************************
 * PRIVATE MACROS AND DEFINES *
 ******************************/

#define MAX_FILEPATH_SIZE       512

/********************
 * PRIVATE TYPEDEFS *
 ********************/

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
    response_callback_t llm_callback;

    llm_status_t status;
} llm_context_t;

/********************
 * STATIC VARIABLES *
 ********************/

static volatile int llm_stop_flag = 0;

/********************
 * STATIC FUNCTIONS *
 ********************/

static result_t create_answer_filepath_from_prompt_filepath( 
        char * answer_filepath, size_t answer_filepath_size, 
        char * prompt_filepath, size_t prompt_filepath_size ) {
    RETURN_IF_NULL(answer_filepath);
    RETURN_ERROR_IF( answer_filepath_size == 0, RES_ERR_WRONG_ARGS );
    RETURN_IF_NULL(prompt_filepath);
    RETURN_ERROR_IF( strlen(prompt_filepath) == 0, RES_ERR_WRONG_ARGS );

    char temp_filepath[prompt_filepath_size];
    strncpy(temp_filepath, prompt_filepath, prompt_filepath_size);
    temp_filepath[sizeof(temp_filepath) - 1] = '\0';

    char * dot = strrchr(temp_filepath, '.');
    RETURN_ERROR_IF( !dot, RES_ERR_GENERIC );

    RETURN_ERROR_IF( (strlen(temp_filepath) + 1) > answer_filepath_size, 
        RES_ERR_GENERIC );

    memcpy(dot, ".out", 4);
    dot[4] = '\0'; 

    strncpy(answer_filepath, temp_filepath, answer_filepath_size);
    answer_filepath[answer_filepath_size - 1] = '\0'; 

    return RES_OK;
}

result_t copy_prompt_to_answer_file( const char * prompt_filepath, 
        const char * answer_filepath ) {
    FILE * src_file = fopen(prompt_filepath, "r");
    FILE * dst_file = fopen(answer_filepath, "w");

    RETURN_IF_NULL(src_file);
    RETURN_IF_NULL(dst_file);

    fprintf(dst_file, "=== PROMPT ===\n\n");

    int ch;
    while( (ch = fgetc(src_file)) != EOF ) {
        fputc(ch, dst_file);
    }

    fprintf(dst_file, "\n=== ANSWER ===\n\n");

    fclose(src_file);
    fclose(dst_file);

    return RES_OK;
}

static void * llm_operation_thread( void * arg ) {
    llm_context_t * params = (llm_context_t *)arg;

    params->status = LLM_STATUS_IN_PROGRESS;
    result_t res = ollama_ask_deepseek_model(params->answer_filepath, params->prompt_filepath, 
        params->llm_stop_flag, params->llm_callback);
    params->status = (res == RES_OK) ? LLM_STATUS_FINISHED_OK : 
                                       LLM_STATUS_FINISHED_ERROR;

    return NULL;
}

static void pipeline_done_event_publish( char * answer_filepath, 
        size_t answer_filepath_size ) {    
    event_t event = STRUCT_INIT_ALL_ZEROS;
    result_t res = event_create(
        COMPONENT_LLM, COMPONENT_CORE_DISP,
        EVENT_PIPELINE_DONE, 
        answer_filepath, answer_filepath_size,
        &event);

    if( res == RES_OK ) {
        broker_publish(&event);
    }
}

static void pipeline_failed_event_publish( void ) {    
    event_t event = STRUCT_INIT_ALL_ZEROS;
    result_t res = event_create(
        COMPONENT_LLM, COMPONENT_CORE_DISP,
        EVENT_PIPELINE_DONE, 
        NULL, 0,
        &event);

    if( res == RES_OK ) {
        broker_publish(&event);
    }
}

static void llm_status_event_publish( const char * status_msg, 
        size_t status_msg_size ) {    
    event_t event = STRUCT_INIT_ALL_ZEROS;
    result_t res = event_create(
        COMPONENT_LLM, COMPONENT_CORE_DISP,
        EVENT_LLM_STATUS, 
        status_msg, status_msg_size,
        &event);

    if( res == RES_OK ) {
        broker_publish(&event);
    }
}

static void llm_context_clear( llm_context_t * context ) {
    memset(context->prompt_filepath, 0, sizeof(context->prompt_filepath));
    memset(context->answer_filepath, 0, sizeof(context->answer_filepath));
    
    llm_stop_flag = 0;

    context->status = LLM_STATUS_NOT_STARTED;
}

static void llm_response_callback( char * data, size_t size, void * user_data ) {
    FILE * output_file = (FILE *)user_data;
    fprintf(output_file, "%.*s", (int)size, data);
    fflush(output_file);

    char msg[EVENT_MAX_DATA_SIZE];
    snprintf(msg, strlen(data) + 1, "%.*s", (int)size, data);
    llm_status_event_publish(msg, strlen(data));
}

/********************
 * GLOBAL FUNCTIONS *
 ********************/

void * llm_thread( void * arg UNUSED_PARAM ) {
    pthread_t llm_op_thread;
    llm_context_t context = {
        .llm_stop_flag = &llm_stop_flag,
        .llm_callback = llm_response_callback,

        .status = LLM_STATUS_NOT_STARTED
    };

    while(1) {
        // Action based on actual operation status
        switch( context.status ) {
            case LLM_STATUS_NOT_STARTED:
                break;

            case LLM_STATUS_IN_PROGRESS: {
                // Sending event with partial answer is in response callback
                break;
            }

            case LLM_STATUS_FINISHED_OK: {
                const char * status_msg = "\nLLM finished.\n";
                llm_status_event_publish(status_msg, 
                    strlen(status_msg));
                pipeline_done_event_publish(context.answer_filepath, 
                    sizeof(context.answer_filepath));
                llm_context_clear(&context);
                break;
            }

            case LLM_STATUS_FINISHED_ERROR: {
                const char * error_msg = "\nError: LLM failed.\n";
                llm_status_event_publish(error_msg, 
                    strlen(error_msg));
                pipeline_failed_event_publish();
                llm_context_clear(&context);
                break;
            }

            default:
                break;
        }

        // Action based on incomming event
        event_t e = STRUCT_INIT_ALL_ZEROS;
        if( broker_pop(COMPONENT_LLM, &e) == RES_OK ) {
            switch( e.type ) {
                case EVENT_LLM_REQUEST: {
                    if( context.status != LLM_STATUS_NOT_STARTED ) {
                        const char * status_msg = 
                            "Ignoring LLM request (already started).\n";
                        llm_status_event_publish(status_msg, 
                            strlen(status_msg));
                        continue;
                    }

                    if( e.data_size > sizeof(context.prompt_filepath) ) {
                        const char * error_msg = 
                            "Error: Failed TXT path in event.\n";
                        llm_status_event_publish(error_msg, 
                            strlen(error_msg));
                        continue;
                    }

                    memcpy(context.prompt_filepath, e.data, e.data_size);

                    result_t res = create_answer_filepath_from_prompt_filepath(
                        context.answer_filepath, sizeof(context.answer_filepath), 
                        context.prompt_filepath, sizeof(context.prompt_filepath));
                    if( res != RES_OK ) {
                        const char * error_msg = 
                            "Error: Failed to create OUT path.\n";
                        llm_status_event_publish(error_msg, 
                            strlen(error_msg));
                        continue;
                    }

                    copy_prompt_to_answer_file(context.prompt_filepath,
                        context.answer_filepath);

                    const char * msg = "LLM start.\n";
                    llm_status_event_publish(msg, strlen(msg));

                    llm_stop_flag = 0;
                    pthread_create(&llm_op_thread, NULL, llm_operation_thread, &context);

                    break;
                }

                case EVENT_LLM_STOP: {
                    if( context.status != LLM_STATUS_IN_PROGRESS ) {
                        const char * status_msg = 
                            "Ignoring stopping LLM request (not started).\n";
                        llm_status_event_publish(status_msg, 
                            strlen(status_msg));
                        continue;
                    }

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
