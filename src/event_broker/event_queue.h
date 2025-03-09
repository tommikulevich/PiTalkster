/**
 *******************************************************************************
 * @file    event_queue.h
 * @brief   Event queue header file.
 *******************************************************************************
 */

#ifndef EVENT_QUEUE_H
#define EVENT_QUEUE_H

#ifdef __cplusplus
extern "C" {
#endif

/************
 * INCLUDES *
 ************/

#include <pthread.h>
#include <stdint.h>

#include "utils.h"

#include "event.h"

/**********************
 * MACROS AND DEFINES *
 **********************/

#define EVENT_QUEUE_SIZE 32

/************
 * TYPEDEFS *
 ************/

typedef struct {
    int head;
    int tail;
    pthread_mutex_t mu;

    event_t events[EVENT_QUEUE_SIZE];
} event_queue_t;

/******************************
 * GLOBAL FUNCTION PROTOTYPES *
 ******************************/

extern result_t event_queue_init( event_queue_t * q );
extern result_t event_queue_push( event_queue_t * q, event_t * e );
extern result_t event_queue_pop( event_queue_t * q, sys_component_t consumer, 
    event_t * event OUTPUT );

#ifdef __cplusplus
}
#endif

#endif /* EVENT_QUEUE_H */
