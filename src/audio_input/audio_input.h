#ifndef AUDIO_INPUT_H
#define AUDIO_INPUT_H

#include "utils.h"

extern result_t audio_input_init( void );
extern void * audio_input_thread( void * arg );

#endif
