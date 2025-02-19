#include <stdint.h>
#include <string.h>
#include <pigpiod_if2.h>

#include "controls_hw.h"

static int pi = -1;
static uint32_t last_tick = 0;

#define DEBOUNCE_TIME_US 2000

static void button_callback( int pi_inst UNUSED_PARAM, unsigned gpio, unsigned level, 
        uint32_t tick, void * user_data ) {

    const action_wrapper_t * action_wrapper = (const action_wrapper_t *)user_data;

    if( (tick - last_tick) < DEBOUNCE_TIME_US )
        return;

    last_tick = tick;

    switch( gpio ) {
        case BUTTON_OK_GPIO: {
            if( level == PI_HIGH ) {
                if( action_wrapper != NULL ) {
                    action_wrapper->action_on_press_button(BUTTON_OK_GPIO);
                }
            }
            break;
        }

        case BUTTON_UP_GPIO: {
            if( level == PI_HIGH ) {
                if( action_wrapper != NULL ) {
                    action_wrapper->action_on_press_button(BUTTON_UP_GPIO);
                }
            }
            break;
        }

        case BUTTON_DOWN_GPIO: {
            if( level == PI_HIGH ) {
                if( action_wrapper != NULL ) {
                    action_wrapper->action_on_press_button(BUTTON_DOWN_GPIO);
                }
            }
            break;
        }

        default:
            break;
    }
}

static result_t gpio_single_init( button_gpio_t button_gpio, 
        const action_wrapper_t * action_wrapper ) {    
    set_mode(pi, button_gpio, PI_INPUT);
    set_pull_up_down(pi, button_gpio, PI_PUD_UP);

    int cb = callback_ex(pi, button_gpio, EITHER_EDGE, button_callback, 
        (void *)action_wrapper);
    if( cb < 0 ) {
         pigpio_stop(pi);
         return RES_ERR_GENERIC;
    }

    return RES_OK;
}

result_t controls_hw_init( const action_wrapper_t * action_wrapper ) {
    pi = pigpio_start(NULL, NULL);
    if( pi < 0 ) return RES_ERR_GENERIC;

    RETURN_ON_ERROR( gpio_single_init(BUTTON_UP_GPIO, action_wrapper) );
    RETURN_ON_ERROR( gpio_single_init(BUTTON_OK_GPIO, action_wrapper) );
    RETURN_ON_ERROR( gpio_single_init(BUTTON_DOWN_GPIO, action_wrapper) );

    return RES_OK;
}
