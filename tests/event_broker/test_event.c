#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "event.h"

static void test_event_create_success( void ** state ) {
    (void) state;

    event_t e;
    const char payload[] = "test_data";
    
    result_t res = event_create(COMPONENT_CONTROLS, COMPONENT_CORE_DISP, 
        EVENT_BUT_PRESSED, 
        payload, sizeof(payload), 
        &e);
    
    assert_int_equal(res, RES_OK);
    assert_int_equal(e.src, COMPONENT_CONTROLS);
    assert_int_equal(e.dest, COMPONENT_CORE_DISP);
    assert_int_equal(e.type, EVENT_BUT_PRESSED);
    assert_int_equal(e.data_size, sizeof(payload));
    assert_memory_equal(e.data, payload, sizeof(payload));
}

static void test_event_create_null_output( void ** state ) {
    (void) state;

    const char payload[] = "test_data";
    
    result_t res = event_create(COMPONENT_CONTROLS, COMPONENT_CORE_DISP, 
        EVENT_BUT_PRESSED, 
        payload, sizeof(payload), 
        NULL);
    
    assert_int_equal(res, RES_ERR_NULL_PTR);
}

static void test_event_create_data_size_too_big( void ** state ) {
    (void) state;

    event_t e;
    uint8_t large_data[EVENT_MAX_DATA_SIZE + 1] = {0};

    result_t res = event_create(COMPONENT_CONTROLS, COMPONENT_CORE_DISP, 
        EVENT_BUT_PRESSED, 
        large_data, sizeof(large_data), 
        &e);
        
    assert_int_equal(res, RES_ERR_INVALID_SIZE);
}

static void test_event_create_no_data( void ** state ) {
    (void) state;

    event_t e;
    result_t res = event_create(COMPONENT_CONTROLS, COMPONENT_CORE_DISP, 
        EVENT_BUT_PRESSED, 
        NULL, 0, 
        &e);
        
    assert_int_equal(res, RES_OK);
    assert_int_equal(e.data_size, 0);
}

int main( void ) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_event_create_success),
        cmocka_unit_test(test_event_create_null_output),
        cmocka_unit_test(test_event_create_data_size_too_big),
        cmocka_unit_test(test_event_create_no_data),
    };
    
    return cmocka_run_group_tests(tests, NULL, NULL);
}
