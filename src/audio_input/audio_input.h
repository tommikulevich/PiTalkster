/**
 *******************************************************************************
 * @file    audio_input.h
 * @brief   Audio input header file.
 *******************************************************************************
 */

#ifndef AUDIO_INPUT_H
#define AUDIO_INPUT_H

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

extern result_t audio_input_init( void );
extern void * audio_input_thread( void * arg );

#ifdef __cplusplus
}
#endif

#endif /* AUDIO_INPUT_H */
