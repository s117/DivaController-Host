//
//  DSC_Reader.cpp
//  DivaController-Host
//
//  Created by s117 on 16/9/30.
//  Copyright © 2016年 s117. All rights reserved.
//

#ifndef __H_DS4_OUTPUT
#define __H_DS4_OUTPUT
#include "stddef.h"

class DS4_Output{
public:
    virtual int write(const char* data, size_t len) = 0;
    virtual void flush() = 0;
};

#endif
