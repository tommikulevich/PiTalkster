/**
 *******************************************************************************
 * @file    stt_ops.h
 * @brief   Speech-to-text operations header file. 
 *******************************************************************************
 */

#ifndef STT_OPS_H
#define STT_OPS_H

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

extern result_t perform_speech_to_text( 
    const char * txt_filepath, const char * wav_filepath,
    volatile int * stop_flag, volatile int * progress );

#ifdef __cplusplus
}
#endif

#endif /* STT_OPS_H */
