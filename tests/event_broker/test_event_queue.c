#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include <pthread.h>
#include <setjmp.h>
#include <cmocka.h>

#include "event_queue.h"

static void test_event_queue_init_success( void ** state ) {
    (void)state;

    event_queue_t q;
    assert_int_equal(event_queue_init(&q), RES_OK);
    assert_int_equal(q.head, 0);
    assert_int_equal(q.tail, 0);
}

static void test_event_queue_push_pop_basic( void ** state ) {
    (void)state;

    event_queue_t q;
    event_queue_init(&q);

    event_t e_push, e_pop;
    event_create(COMPONENT_CONTROLS, COMPONENT_CORE_DISP, 
        EVENT_BUT_PRESSED, 
        "test", 5, 
        &e_push);
    assert_int_equal(event_queue_push(&q, &e_push), RES_OK);
    assert_int_equal(event_queue_pop(&q, COMPONENT_CORE_DISP, &e_pop), RES_OK);
    assert_memory_equal(&e_push, &e_pop, sizeof(event_t));
}

static void test_event_queue_push_full_queue( void ** state ) {
    (void)state;

    event_queue_t q;
    event_queue_init(&q);

    event_t e;
    for( size_t i = 0; i < EVENT_QUEUE_SIZE - 1; i++ ) {
        event_create(COMPONENT_CONTROLS, COMPONENT_CORE_DISP, 
            EVENT_BUT_PRESSED, 
            &i, sizeof(i), 
            &e);
        assert_int_equal(event_queue_push(&q, &e), RES_OK);
    }

    event_create(COMPONENT_CONTROLS, COMPONENT_CORE_DISP, 
        EVENT_BUT_PRESSED, 
        NULL, 0, 
        &e);
    assert_int_equal(event_queue_push(&q, &e), RES_ERR_GENERIC);
}

static void test_event_queue_pop_empty_queue( void ** state ) {
    (void)state;

    event_queue_t q;
    event_queue_init(&q);

    event_t e;
    assert_int_equal(event_queue_pop(&q, COMPONENT_CORE_DISP, &e), RES_ERR_GENERIC);
}

static void test_event_queue_push_pop_randomized( void ** state ) {
    (void)state;

    event_queue_t q;
    event_queue_init(&q);

    srand((unsigned int)time(NULL));

    event_t e_push, e_pop;
    for( int i = 0; i < 1000; i++ ) {
        sys_component_t src = rand() % 2;
        sys_component_t dest = rand() % 2;
        event_type_t type = rand() % 2;
        uint8_t data[EVENT_MAX_DATA_SIZE];
        size_t data_size = rand() % sizeof(data);
        for( size_t j = 0; j < data_size; j++ ) {
            data[j] = rand() % 256;
        }

        event_create(src, dest, type, data, data_size, &e_push);
        event_queue_push(&q, &e_push);

        int pop_result = event_queue_pop(&q, dest, &e_pop);
        assert_int_equal(pop_result, RES_OK);
        assert_memory_equal(&e_push, &e_pop, sizeof(event_t));
    }
}

static void * producer_thread( void * arg ) {
    event_queue_t * q = arg;
    event_t e;
    for( int i = 0; i < 10000; i++ ) {
        event_create(COMPONENT_CONTROLS, COMPONENT_CORE_DISP, 
            EVENT_BUT_PRESSED, 
            &i, sizeof(i), 
            &e);
        while( event_queue_push(q, &e) != RES_OK );
    }

    return NULL;
}

static void * consumer_thread( void * arg ) {
    event_queue_t * q = arg;
    event_t e;
    int count = 0;
    while( count < 10000 ) {
        if( event_queue_pop(q, COMPONENT_CORE_DISP, &e) == RES_OK ) {
            count++;
        }   
    }

    return NULL;
}

static void test_event_queue_multithreaded( void ** state ) {
    (void)state;

    pthread_t prod_thread, cons_thread;
    event_queue_t q;
    event_queue_init(&q);

    pthread_create(&prod_thread, NULL, producer_thread, &q);
    pthread_create(&cons_thread, NULL, consumer_thread, &q);

    pthread_join(prod_thread, NULL);
    pthread_join(cons_thread, NULL);

    assert_int_equal(q.head, q.tail);
}

int main( void ) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_event_queue_init_success),
        cmocka_unit_test(test_event_queue_push_pop_basic),
        cmocka_unit_test(test_event_queue_push_full_queue),
        cmocka_unit_test(test_event_queue_pop_empty_queue),
        cmocka_unit_test(test_event_queue_push_pop_randomized),
        cmocka_unit_test(test_event_queue_multithreaded),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
