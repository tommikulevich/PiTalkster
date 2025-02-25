#include "utils.h"

#include "display.h"
#include "display_hw.h"

result_t display_init(void) {
    return display_hw_init();
}
