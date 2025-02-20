#include <stdint.h>
#include <string.h>

#include "controls.h"
#include "controls_hw.h"
#include "event_broker.h"

void button_event_publish( button_gpio_t button_gpio ) {
    event_t event = STRUCT_INIT_ALL_ZEROS;
    result_t create_res = event_create(
        COMPONENT_CONTROLS, COMPONENT_CORE_DISP,
        EVENT_BUT_PRESSED, 
        &button_gpio, sizeof(button_gpio_t),
        &event);

    if( create_res != RES_OK ) {
        ERROR("Failed to create event %s for button %s: error %d", 
            event_type_enum_to_string(EVENT_BUT_PRESSED),
            button_gpio_enum_to_string(button_gpio),
            create_res);
        return;
    }
    
    INFO("%s pressed! Publish event %s", 
        button_gpio_enum_to_string(button_gpio),
        event_type_enum_to_string(EVENT_BUT_PRESSED));
    broker_publish(&event);
}

const action_wrapper_t action_wrapper = { button_event_publish };

result_t controls_init(void) {
    return controls_hw_init(&action_wrapper);
}
