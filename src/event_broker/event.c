#include <string.h>

#include "utils.h"

#include "event.h"

result_t event_create( sys_component_t src, sys_component_t dest, 
        event_type_t type, const void * data, size_t data_size,
        event_t * event OUTPUT ) {
    ASSERT_NOT_NULL(event);

    if( data_size > EVENT_MAX_DATA_SIZE ) {
        ERROR("Failed to create event %s: data size %ld > %d!",
            event_type_enum_to_string(type), data_size, EVENT_MAX_DATA_SIZE);
        return RES_ERR_INVALID_SIZE;
    }

    event->src = src;
    event->dest = dest;
    event->type = type;
    event->data_size = data_size;

    if( data && data_size > 0 ) {
        memcpy(event->data, data, data_size);
    }

    return RES_OK;
}
