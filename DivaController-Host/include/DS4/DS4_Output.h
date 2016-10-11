// Copyright (C) 2016 S117 <admin@0x10c.pw>
// Bitman Lab.
// Licensed under the terms of the GNU GPL, version 2
// http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt


#ifndef __H_DS4_OUTPUT
#define __H_DS4_OUTPUT
#include "stddef.h"

class DS4_Output {
public:
    virtual int write(const char* data, size_t len) = 0;
    virtual void flush() = 0;
};

#endif
