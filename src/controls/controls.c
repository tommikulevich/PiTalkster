#include <stdint.h>
#include <string.h>

#include "controls.h"
#include "controls_hw.h"
#include "controls_gpio.h"
#include "event_broker.h"

static void button_event_publish( button_gpio_t button_gpio ) {
    event_t event = STRUCT_INIT_ALL_ZEROS;
    result_t res = event_create(
        COMPONENT_CONTROLS, COMPONENT_CORE_DISP,
        EVENT_BUT_PRESSED, 
        &button_gpio, sizeof(button_gpio_t),
        &event);

    if( res == RES_OK ) {
        broker_publish(&event);
    }
}

result_t controls_init(void) {
    RETURN_ON_ERROR( controls_hw_init_button(BUTTON_UP_GPIO, button_event_publish) );
    RETURN_ON_ERROR( controls_hw_init_button(BUTTON_OK_GPIO, button_event_publish) );
    RETURN_ON_ERROR( controls_hw_init_button(BUTTON_DOWN_GPIO, button_event_publish) );
    
    return RES_OK;
}
