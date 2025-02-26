#ifndef AUDIO_INPUT_REC_OPS_H
#define AUDIO_INPUT_REC_OPS_H

#include "utils.h"

extern result_t record_audio( const char * filepath OUTPUT, int duration_s, 
    volatile int * stop_flag );

#endif
