#include <pthread.h>
#include <unistd.h>

#include "utils.h"

#include "controls.h"
#include "audio_input.h"
#include "display.h"
#include "event_broker.h"
#include "core.h"
#include "stt.h"


int main( int argc UNUSED_PARAM, char *argv[] UNUSED_PARAM ) {
    // HW
    ASSERT( controls_init() == RES_OK );
    ASSERT( display_init() == RES_OK );
    ASSERT( audio_input_init() == RES_OK );

    // SW
    ASSERT( broker_init() == RES_OK );
    ASSERT( core_init() == RES_OK );
    ASSERT( stt_init() == RES_OK );

    // Audio input thread
    pthread_t thr_audio;
    pthread_create(&thr_audio, NULL, audio_input_thread, NULL);

    // STT thread
    pthread_t thr_stt;
    pthread_create(&thr_stt, NULL, stt_thread, NULL);

    // Main core thread
    pthread_t thr_core;
    pthread_create(&thr_core, NULL, core_thread, NULL);
    pthread_join(thr_core, NULL); 

    return EXIT_FAILURE;    // Something wrong happend if we reached this
}