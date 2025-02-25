#include "utils.h"

#include "event.h"
#include "event_queue.h"

result_t event_queue_init( event_queue_t * q ) {
    ASSERT_NOT_NULL(q);

    q->head = 0;
    q->tail = 0;

    if( pthread_mutex_init(&q->mu, NULL) != 0 ) {
        return RES_ERR_NOT_READY;
    }

    return RES_OK;
}

result_t event_queue_push( event_queue_t * q, event_t * e ) {
    ASSERT_NOT_NULL(q);
    ASSERT_NOT_NULL(e);

    pthread_mutex_lock(&q->mu);

    int next_tail = (q->tail + 1) % EVENT_QUEUE_SIZE;
    if( next_tail == q->head ) {
        pthread_mutex_unlock(&q->mu);
        return RES_ERR_GENERIC; 
    }
    q->events[q->tail] = *e;
    q->tail = next_tail;

    pthread_mutex_unlock(&q->mu);

    return RES_OK;
}

result_t event_queue_pop( event_queue_t * q, sys_component_t consumer, 
        event_t * event OUTPUT ) {
    ASSERT_NOT_NULL(q);
    ASSERT_NOT_NULL(event);

    pthread_mutex_lock(&q->mu);

    int idx = q->head;
    while( idx != q->tail ) {
        if( q->events[idx].dest == consumer ) {
            *event = q->events[idx];
            int move_idx = idx;
            while( move_idx != q->tail ) {
                int nxt = (move_idx + 1) % EVENT_QUEUE_SIZE;
                if( nxt == q->tail ) break;
                q->events[move_idx] = q->events[nxt];
                move_idx = nxt;
            }
            q->tail = (q->tail + EVENT_QUEUE_SIZE - 1) % EVENT_QUEUE_SIZE;
            pthread_mutex_unlock(&q->mu);
            return RES_OK;
        }
        idx = (idx + 1) % EVENT_QUEUE_SIZE;
    }

    pthread_mutex_unlock(&q->mu);

    return RES_ERR_GENERIC; 
}
