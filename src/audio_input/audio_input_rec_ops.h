/**
 *******************************************************************************
 * @file    audio_input_rec_ops.h
 * @brief   Recording operations header file. 
 *******************************************************************************
 */

#ifndef AUDIO_INPUT_REC_OPS_H
#define AUDIO_INPUT_REC_OPS_H

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

extern result_t record_audio_to_wav( const char * wav_filepath, int duration_s, 
    volatile int * stop_flag, volatile int * progress );

#ifdef __cplusplus
}
#endif

#endif /* AUDIO_INPUT_REC_OPS_H */
