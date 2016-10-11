// Copyright (C) 2016 S117 <admin@0x10c.pw>
// Bitman Lab.
// Licensed under the terms of the GNU GPL, version 2
// http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt


#include "DS4_Output_STDOUT.h"
#include <stdio.h>

int DS4_Output_STDOUT::write(const char* data, size_t len) {
    for(int i = 0; i<len; ++i) {
        fputc(data[i], stdout);
    }
    return 0;
}

void DS4_Output_STDOUT::flush() {
    fflush(stdout);
}


