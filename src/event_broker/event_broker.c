#include "utils.h"

#include "event.h"
#include "event_queue.h"
#include "event_broker.h"

static event_queue_t g_queue;

result_t broker_init( void ) {
    return event_queue_init(&g_queue);
}

result_t broker_publish( event_t * e ) {
    ASSERT_NOT_NULL(e);
    
    INFO(CYAN"[⇧] PUBLISH EVENT \'%s\' FROM [%s] TO [%s]. DATA SIZE: %ld"RST, 
        event_type_enum_to_string(e->type),
        sys_component_enum_to_string(e->src),
        sys_component_enum_to_string(e->dest),
        e->data_size);

    result_t res = event_queue_push(&g_queue, e);
    if( res != RES_OK ) {
        ERROR("Failed to push event into queue. Error code: %d", res);
    }

    return res;
}

result_t broker_pop( sys_component_t c, event_t * e OUTPUT ) {
    ASSERT_NOT_NULL(e);

    result_t res = event_queue_pop(&g_queue, c, e);
    if( res != RES_OK ) {
        // There is no event for selected component
        return res;
    }

    INFO(MAGENTA"[⇩] POP EVENT \'%s\' FROM [%s] BY [%s]. DATA SIZE: %ld"RST, 
        sys_component_enum_to_string(c),
        event_type_enum_to_string(e->type),
        sys_component_enum_to_string(e->src),
        e->data_size);

    return res;
}
