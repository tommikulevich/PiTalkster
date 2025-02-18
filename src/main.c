#include <pthread.h>
#include <unistd.h>

#include "utils.h"

#include "controls.h"
#include "event_broker.h"
#include "core.h"


int main( int argc UNUSED_PARAM, char *argv[] UNUSED_PARAM ) {
    // HW
    ASSERT( controls_init() == RES_OK );

    // SW
    ASSERT( broker_init() == RES_OK );
    ASSERT( core_init() == RES_OK );

    // Main core thread
    pthread_t thr_core;
    pthread_create(&thr_core, NULL, core_thread, NULL);
    pthread_join(thr_core, NULL); 

    return EXIT_FAILURE;    // Something wrong happend if we reached this
}