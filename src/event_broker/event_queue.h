#ifndef EVENT_QUEUE_H
#define EVENT_QUEUE_H

#include <pthread.h>
#include <stdint.h>

#include "utils.h"

#define EVENT_QUEUE_SIZE 32
#define EVENT_DATA_SIZE 4096

typedef enum {
    COMPONENT_CORE_DISP,
    COMPONENT_CONTROLS,
    COMPONENT_AUDIO_INPUT,
    COMPONENT_STT,
    COMPONENT_LLM
} sys_component_t;

static inline const char* sys_component_enum_to_string( sys_component_t sys_component ) {
    switch( sys_component ) {
        case COMPONENT_CORE_DISP:   return "COMPONENT_CORE_DISP";
        case COMPONENT_CONTROLS:    return "COMPONENT_CONTROLS";
        case COMPONENT_AUDIO_INPUT: return "COMPONENT_AUDIO_INPUT";
        case COMPONENT_STT:         return "COMPONENT_STT";
        case COMPONENT_LLM:         return "COMPONENT_LLM";
        
        default:                    return "COMPONENT_UNDEFINED";
    }
}

typedef enum {
    EVENT_BUT_PRESSED,

    EVENT_REC_REQUEST,
    EVENT_REC_CANCEL,
    EVENT_REC_ERROR,

    EVENT_STT_REQUEST,
    EVENT_STT_CANCEL,
    EVENT_STT_ERROR,

    EVENT_LLM_REQUEST,
    EVENT_LLM_CANCEL,
    EVENT_LLM_ERROR,

    EVENT_PIPELINE_DONE,
    EVENT_DISPLAY_UPDATE
} event_type_t;

static inline const char* event_type_enum_to_string( event_type_t event_type ) {
    switch( event_type ) {
        case EVENT_BUT_PRESSED:     return "EVENT_BUT_PRESSED";

        case EVENT_REC_REQUEST:     return "EVENT_REC_REQUEST";
        case EVENT_REC_CANCEL:      return "EVENT_REC_CANCEL";
        case EVENT_REC_ERROR:       return "EVENT_REC_ERROR";

        case EVENT_STT_REQUEST:     return "EVENT_STT_REQUEST";
        case EVENT_STT_CANCEL:      return "EVENT_STT_CANCEL";
        case EVENT_STT_ERROR:       return "EVENT_STT_ERROR";

        case EVENT_LLM_REQUEST:     return "EVENT_LLM_REQUEST";
        case EVENT_LLM_CANCEL:      return "EVENT_LLM_CANCEL";
        case EVENT_LLM_ERROR:       return "EVENT_LLM_ERROR";

        case EVENT_PIPELINE_DONE:   return "EVENT_PIPELINE_DONE";
        case EVENT_DISPLAY_UPDATE:  return "EVENT_DISPLAY_UPDATE";
        
        default:                    return "EVENT_UNDEFINED";
    }
}

typedef struct {
    sys_component_t src;
    sys_component_t dest;
    event_type_t event_type;

    uint8_t data[EVENT_DATA_SIZE];
    size_t data_size;
} event_t;

typedef struct {
    int head;
    int tail;
    pthread_mutex_t mu;

    event_t events[EVENT_QUEUE_SIZE];
} event_queue_t;

extern result_t event_create( sys_component_t src, sys_component_t dest, 
    event_type_t event_type, const void * data, size_t data_size,
    event_t * event OUTPUT );
extern result_t event_queue_init( event_queue_t * q );
extern result_t event_queue_push( event_queue_t * q, event_t * e );
extern result_t event_queue_pop( event_queue_t * q, sys_component_t consumer, 
    event_t * event OUTPUT );

#endif
