#ifndef STT_OPS_H
#define STT_OPS_H

#include "utils.h"

extern result_t perform_speech_to_text( const char * txt_filepath OUTPUT, 
    const char * wav_filepath,
    volatile int * stop_flag );

#endif
