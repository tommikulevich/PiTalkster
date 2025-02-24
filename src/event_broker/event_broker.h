#ifndef EVENT_BROKER_H
#define EVENT_BROKER_H

#include "utils.h"

#include "event_queue.h"

extern result_t broker_init( void);
extern result_t broker_publish( event_t * e );
extern result_t broker_pop( sys_component_t c, event_t * e OUTPUT );

#endif
