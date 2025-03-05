#ifndef OLLAMA_API_OPS_H
#define OLLAMA_API_OPS_H

#include "utils.h"

extern result_t ollama_ask_deepseek_model( const char * answer_filepath OUTPUT, 
    const char * prompt_filepath,
    volatile int * stop_flag );

#endif
