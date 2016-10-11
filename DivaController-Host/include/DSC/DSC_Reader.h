// Copyright (C) 2016 S117 <admin@0x10c.pw>
// Bitman Lab.
// Licensed under the terms of the GNU GPL, version 2
// http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt


#ifndef __H_DSC_READER
#define __H_DSC_READER

#include <stdio.h>
#include "PVSC_Parser.h"

class DSC_Reader {
public:
    static DSC_Info* read_dsc(const char* path);
};

#endif /* DSC_Reader_hpp */
