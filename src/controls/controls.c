#include <stdint.h>
#include <string.h>
#include <pigpiod_if2.h>

#include "controls.h"
#include "event_broker.h"

static int pi = -1;
static uint32_t last_tick = 0;

#define DEBOUNCE_TIME_US 2000


static void button_event_publish( button_gpio_t button_gpio ) {
    event_t event = STRUCT_INIT_ALL_ZEROS;
    result_t create_res = event_create(
        COMPONENT_CONTROLS, COMPONENT_CORE,
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

static void button_callback( int pi_inst UNUSED_PARAM, unsigned gpio, unsigned level, 
        uint32_t tick, void * user UNUSED_PARAM ) {

    if( (tick - last_tick) < DEBOUNCE_TIME_US )
        return;

    last_tick = tick;

    switch( gpio ) {
        case BUTTON_OK_GPIO: {
            if( level == PI_HIGH ) {
                button_event_publish(BUTTON_OK_GPIO);
            }
            break;
        }

        case BUTTON_UP_GPIO: {
            if( level == PI_HIGH ) {
                button_event_publish(BUTTON_UP_GPIO);
            }
            break;
        }

        case BUTTON_DOWN_GPIO: {
            if( level == PI_HIGH ) {
                button_event_publish(BUTTON_DOWN_GPIO);
            }
            break;
        }

        default:
            break;
    }
}

static result_t gpio_init( button_gpio_t button_gpio ) {
    set_mode(pi, button_gpio, PI_INPUT);
    set_pull_up_down(pi, button_gpio, PI_PUD_UP);

    int cb = callback_ex(pi, button_gpio, EITHER_EDGE, button_callback, NULL);
    if( cb < 0 ) {
         pigpio_stop(pi);
         return RES_ERR_GENERIC;
    }

    return RES_OK;
}

result_t controls_init(void) {
    pi = pigpio_start(NULL, NULL);
    if( pi < 0 ) return RES_ERR_GENERIC;

    RETURN_ON_ERROR( gpio_init(BUTTON_UP_GPIO) );
    RETURN_ON_ERROR( gpio_init(BUTTON_OK_GPIO) );
    RETURN_ON_ERROR( gpio_init(BUTTON_DOWN_GPIO) );

    return RES_OK;
}
