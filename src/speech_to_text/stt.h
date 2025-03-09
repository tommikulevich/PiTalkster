/**
 *******************************************************************************
 * @file    stt.h
 * @brief   Speech-to-text header file.
 *******************************************************************************
 */

#ifndef STT_H
#define STT_H

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

extern result_t stt_init( void );
extern void * stt_thread( void * arg );

#ifdef __cplusplus
}
#endif

#endif /* STT_H */
