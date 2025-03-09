/**
 *******************************************************************************
 * @file    llm.h
 * @brief   Large language model interaction header file.
 *******************************************************************************
 */

#ifndef LLM_H
#define LLM_H

#ifdef __cplusplus
extern "C" {
#endif

/************
 * INCLUDES *
 ************/

#include "utils.h"

/******************************
 * GLOBAL FUNCTION PROTOTYPES *
 ******************************/

extern result_t llm_init( void );
extern void * llm_thread( void * arg );

#ifdef __cplusplus
}
#endif

#endif /* LLM_H */
