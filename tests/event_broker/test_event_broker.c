#include <stdarg.h>
#include <stddef.h>
#include <assert.h>
#include <setjmp.h>
#include <cmocka.h>

#include "event_queue.h"
#include "event_broker.h"

static int setup( void ** state ) {
    (void) state;
    assert_int_equal(broker_init(), RES_OK);
    return 0;
}

static void test_broker_publish_and_pop_success( void ** state ) {
    (void) state;

    event_t evt_pub, evt_sub;
    event_create(COMPONENT_CONTROLS, COMPONENT_CORE_DISP, 
        EVENT_BUT_PRESSED, 
        "data", 5, 
        &evt_pub);
    assert_int_equal(broker_publish(&evt_pub), RES_OK);
    assert_int_equal(broker_pop(COMPONENT_CORE_DISP, &evt_sub), RES_OK);
    assert_memory_equal(&evt_pub, &evt_sub, sizeof(event_t));
}

static void test_broker_pop_no_event( void ** state ) {
    (void) state;

    event_t e;
    assert_int_equal(broker_pop(COMPONENT_CORE_DISP, &e), RES_ERR_GENERIC);
}

static void test_broker_null_event_publish( void ** state ) {
    (void) state;
    assert_int_equal(broker_publish(NULL), RES_ERR_NULL_PTR);
}

static void test_broker_null_event_pop( void ** state ) {
    (void) state;
    assert_int_equal(broker_pop(COMPONENT_CONTROLS, NULL), RES_ERR_NULL_PTR);
}

static void test_broker_publish_event_queue_full( void ** state ) {
    (void) state;

    event_t e;
    for( int i = 0; i < EVENT_QUEUE_SIZE - 1; i++ ) {
        event_create(COMPONENT_CONTROLS, COMPONENT_CORE_DISP, EVENT_BUT_PRESSED, NULL, 0, &e);
        assert_int_equal(broker_publish(&e), RES_OK);
    }
    event_create(COMPONENT_CONTROLS, COMPONENT_CORE_DISP, EVENT_BUT_PRESSED, NULL, 0, &e);
    assert_int_equal(broker_publish(&e), RES_ERR_GENERIC);
}

int main( void ) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup(test_broker_publish_and_pop_success, setup),
        cmocka_unit_test_setup(test_broker_pop_no_event, setup),
        cmocka_unit_test_setup(test_broker_null_event_publish, setup),
        cmocka_unit_test_setup(test_broker_null_event_pop, setup),
        cmocka_unit_test_setup(test_broker_publish_event_queue_full, setup),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
