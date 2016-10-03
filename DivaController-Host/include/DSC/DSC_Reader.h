//
//  DSC_Reader.hpp
//  DivaController-Host
//
//  Created by s117 on 16/9/30.
//  Copyright © 2016年 s117. All rights reserved.
//

#ifndef __H_DSC_READER
#define __H_DSC_READER

#include <stdio.h>
#include "PVSC_Parser.h"

class DSC_Reader {
public:
    static DSC_Info* read_dsc(const char* path);
};

#endif /* DSC_Reader_hpp */
