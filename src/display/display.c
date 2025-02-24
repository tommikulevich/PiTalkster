#include <stdint.h>
#include <string.h>

#include "utils.h"

#include "display.h"
#include "display_hw.h"
#include "event_broker.h"

result_t display_init(void) {
    return display_hw_init();
}
