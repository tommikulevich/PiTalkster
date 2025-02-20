#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/select.h>
#include <gpiod.h>

#include "controls_hw.h"

#define DEBOUNCE_TIME_US    2000
#define GPIO_CHIP_PATH      "/dev/gpiochip0"

struct button_info_t {
    button_gpio_t gpio;
    button_handler_t handler;
    struct gpiod_line * line;
    pthread_t thread_id;

    uint64_t last_event_time;
    struct button_info_t * next;
};

static struct gpiod_chip * chip;
static struct button_info_t * buttons_list = NULL;

static void * button_event_thread( void * arg ) {
    struct button_info_t * info = (struct button_info_t *)arg;
    struct gpiod_line_event event;
    uint64_t current_time;

    while(1) {
        int ret = gpiod_line_event_wait(info->line, NULL);
        if( ret == 1 ) {
            ret = gpiod_line_event_read(info->line, &event);
            if( ret == 0 ) {
                current_time = get_current_time_us();
                if( current_time - info->last_event_time >= DEBOUNCE_TIME_US ) {
                    info->last_event_time = current_time;
                    if( event.event_type == GPIOD_LINE_EVENT_RISING_EDGE ) {
                        info->handler(info->gpio);
                    }
                }
            }
        }
    }
    
    return NULL;
}

result_t controls_hw_init_button( button_gpio_t gpio, button_handler_t handler ) {
    if( !chip ) {
        chip = gpiod_chip_open(GPIO_CHIP_PATH);
        if( !chip ) {
            return RES_ERR_NOT_READY;
        }
    }

    struct button_info_t * info = malloc(sizeof(struct button_info_t));
    if( !info ) {
        return RES_ERR_NOT_READY;
    }

    info->gpio = gpio;
    info->handler = handler;
    info->last_event_time = 0;

    info->line = gpiod_chip_get_line(chip, (unsigned int)gpio);
    if( !info->line ) {
        free(info);
        return RES_ERR_GENERIC;
    }

    char label[32];
    snprintf(label, sizeof(label), "button_%d", gpio);

    struct gpiod_line_request_config config = {
        .consumer = label,
        .request_type = GPIOD_LINE_REQUEST_EVENT_RISING_EDGE,
        .flags = GPIOD_LINE_REQUEST_FLAG_BIAS_PULL_UP
    };

    if( gpiod_line_request(info->line, &config, 0) < 0 ) {
        free(info);
        return RES_ERR_GENERIC;
    }

    if( pthread_create(&info->thread_id, NULL, button_event_thread, info) != 0 ) {
        gpiod_line_release(info->line);
        free(info);
        return RES_ERR_GENERIC;
    }

    info->next = buttons_list;
    buttons_list = info;

    return RES_OK;
}
