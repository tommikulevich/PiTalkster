/**
 *******************************************************************************
 * @file    event_broker.h
 * @brief   Event broker header file.
 *******************************************************************************
 */

#ifndef EVENT_BROKER_H
#define EVENT_BROKER_H

#ifdef __cplusplus
extern "C" {
#endif

/************
 * INCLUDES *
 ************/

#include "utils.h"

#include "event.h"

/******************************
 * GLOBAL FUNCTION PROTOTYPES *
 ******************************/

extern result_t broker_init( void);
extern result_t broker_publish( event_t * e );
extern result_t broker_pop( sys_component_t c, event_t * e OUTPUT );

#ifdef __cplusplus
}
#endif

#endif /* EVENT_BROKER_H */
