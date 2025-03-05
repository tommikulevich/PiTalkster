/**
 *******************************************************************************
 * @file    ollama_api_ops.h
 * @brief   Ollama API operations header file.
 *******************************************************************************
 */

#ifndef OLLAMA_API_OPS_H
#define OLLAMA_API_OPS_H

#ifdef __cplusplus
extern "C" {
#endif

/************
 * INCLUDES *
 ************/ 

#include "utils.h"

/************
 * TYPEDEFS *
 ************/

typedef void (*response_callback_t)(char * data, size_t size, 
    void * user_data);

/******************************
 * GLOBAL FUNCTION PROTOTYPES *
 ******************************/

extern result_t ollama_ask_deepseek_model( 
    const char * answer_filepath, const char * prompt_filepath,
    volatile int * stop_flag, response_callback_t callback );

#ifdef __cplusplus
}
#endif

#endif /* OLLAMA_API_OPS_H */
