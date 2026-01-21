#include <stdio.h>
#include <stdint.h>
#include "fixed_point.h"


int main(int argc, char **argv) {
    if (argc != 6) {
        printf("Usage: %s <x_raw> <a_raw> <b_raw> <c_raw> <q>\n", argv[0]);
        printf("All inputs must be integers. (x/a/b/c/q are int16 raw fixed-point values)\n");
        return 0;
    }

    // TODO

    return 0;
}
