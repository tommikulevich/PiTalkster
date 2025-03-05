/**
 *******************************************************************************
 * @file    event.h
 * @brief   Event header file.
 *******************************************************************************
 */

#ifndef EVENT_H
#define EVENT_H

#ifdef __cplusplus
extern "C" {
#endif

/************
 * INCLUDES *
 ************/

#include <stdint.h>

#include "utils.h"

/**********************
 * MACROS AND DEFINES *
 **********************/

#define EVENT_MAX_DATA_SIZE 512

/*******************************
 * TYPEDEFS AND STATIC INLINES *
 *******************************/

typedef enum {
    COMPONENT_CORE_DISP,
    COMPONENT_CONTROLS,
    COMPONENT_AUDIO_INPUT,
    COMPONENT_STT,
    COMPONENT_LLM
} sys_component_t;

static inline const char * sys_component_enum_to_string( sys_component_t sys_component ) {
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
    EVENT_REC_STOP,
    EVENT_REC_STATUS,

    EVENT_STT_REQUEST,
    EVENT_STT_STOP,
    EVENT_STT_STATUS,

    EVENT_LLM_REQUEST,
    EVENT_LLM_STOP,
    EVENT_LLM_STATUS,

    EVENT_PIPELINE_DONE
} event_type_t;

static inline const char* event_type_enum_to_string( event_type_t type ) {
    switch( type ) {
        case EVENT_BUT_PRESSED:     return "BUT_PRESSED";

        case EVENT_REC_REQUEST:     return "REC_REQUEST";
        case EVENT_REC_STOP:      return "REC_STOP";
        case EVENT_REC_STATUS:       return "REC_STATUS";

        case EVENT_STT_REQUEST:     return "STT_REQUEST";
        case EVENT_STT_STOP:      return "STT_STOP";
        case EVENT_STT_STATUS:       return "STT_STATUS";

        case EVENT_LLM_REQUEST:     return "LLM_REQUEST";
        case EVENT_LLM_STOP:      return "LLM_STOP";
        case EVENT_LLM_STATUS:       return "LLM_STATUS";

        case EVENT_PIPELINE_DONE:   return "PIPELINE_DONE";
        
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

/******************************
 * GLOBAL FUNCTION PROTOTYPES *
 ******************************/

extern result_t event_create( sys_component_t src, sys_component_t dest, 
    event_type_t type, const void * data, size_t data_size,
    event_t * event OUTPUT );

#ifdef __cplusplus
}
#endif

#endif /* EVENT_H */
