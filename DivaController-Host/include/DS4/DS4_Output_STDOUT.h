// Copyright (C) 2016 S117 <admin@0x10c.pw>
// Bitman Lab.
// Licensed under the terms of the GNU GPL, version 2
// http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt


#ifndef __H_DS4_OUTPUT_STDOUT
#define __H_DS4_OUTPUT_STDOUT
#include "DS4_Output.h"

class DS4_Output_STDOUT : public DS4_Output {
public:
    virtual int write(const char* data, size_t len);
    virtual void flush();
};

#endif
