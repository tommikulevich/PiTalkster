#ifndef EVENT_QUEUE_H
#define EVENT_QUEUE_H

#include <pthread.h>
#include <stdint.h>

#include "utils.h"

#include "event.h"

#define EVENT_QUEUE_SIZE 32

typedef struct {
    int head;
    int tail;
    pthread_mutex_t mu;

    event_t events[EVENT_QUEUE_SIZE];
} event_queue_t;

extern result_t event_queue_init( event_queue_t * q );
extern result_t event_queue_push( event_queue_t * q, event_t * e );
extern result_t event_queue_pop( event_queue_t * q, sys_component_t consumer, 
    event_t * event OUTPUT );

#endif
