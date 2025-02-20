#include "event_broker.h"

static event_queue_t g_queue;

result_t broker_init( void ) {
    return event_queue_init(&g_queue);
}

result_t broker_publish( event_t * e ) {
    ASSERT_NOT_NULL(e);
    return event_queue_push(&g_queue, e);
}

result_t broker_pop( sys_component_t c, event_t * e OUTPUT ) {
    ASSERT_NOT_NULL(e);
    return event_queue_pop(&g_queue, c, e);
}
