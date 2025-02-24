#include <stdint.h>
#include <string.h>

#include "utils.h"

#include "driver_st7789_basic.h"

result_t display_hw_init(void) {
    if( st7789_basic_init() == 0 ) {
        return st7789_basic_clear();
    }

    return RES_ERR_NOT_READY;
}
