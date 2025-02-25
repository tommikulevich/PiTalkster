#ifndef EVENT_H
#define EVENT_H

#include <stdint.h>

#include "utils.h"

#define EVENT_MAX_DATA_SIZE 512

typedef enum {
    COMPONENT_CORE_DISP,
    COMPONENT_CONTROLS,
    COMPONENT_AUDIO_INPUT,
    COMPONENT_STT,
    COMPONENT_LLM
} sys_component_t;

static inline const char* sys_component_enum_to_string( sys_component_t sys_component ) {
    switch( sys_component ) {
        case COMPONENT_CORE_DISP:   return "CORE_DISPLAY";
        case COMPONENT_CONTROLS:    return "CONTROLS";
        case COMPONENT_AUDIO_INPUT: return "AUDIO_INPUT";
        case COMPONENT_STT:         return "STT";
        case COMPONENT_LLM:         return "LLM";
        
        default:                    return "UNDEFINED";
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

static inline const char* event_type_enum_to_string( event_type_t type ) {
    switch( type ) {
        case EVENT_BUT_PRESSED:     return "BUT_PRESSED";

        case EVENT_REC_REQUEST:     return "REC_REQUEST";
        case EVENT_REC_CANCEL:      return "REC_CANCEL";
        case EVENT_REC_ERROR:       return "REC_ERROR";

        case EVENT_STT_REQUEST:     return "STT_REQUEST";
        case EVENT_STT_CANCEL:      return "STT_CANCEL";
        case EVENT_STT_ERROR:       return "STT_ERROR";

        case EVENT_LLM_REQUEST:     return "LLM_REQUEST";
        case EVENT_LLM_CANCEL:      return "LLM_CANCEL";
        case EVENT_LLM_ERROR:       return "LLM_ERROR";

        case EVENT_PIPELINE_DONE:   return "PIPELINE_DONE";
        case EVENT_DISPLAY_UPDATE:  return "DISPLAY_UPDATE";
        
        default:                    return "UNDEFINED";
    }
}

typedef struct {
    sys_component_t src;
    sys_component_t dest;
    event_type_t type;

    uint8_t data[EVENT_MAX_DATA_SIZE];
    size_t data_size;
} event_t;

extern result_t event_create( sys_component_t src, sys_component_t dest, 
    event_type_t type, const void * data, size_t data_size,
    event_t * event OUTPUT );

#endif
