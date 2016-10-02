//
//  DSC_Reader.cpp
//  DivaController-Host
//
//  Created by s117 on 16/9/30.
//  Copyright © 2016年 s117. All rights reserved.
//

#ifndef __H_DS4_OUTPUT_STDOUT
#define __H_DS4_OUTPUT_STDOUT
#include "DS4_Output.h"

class DS4_Output_STDOUT : public DS4_Output{
public:
    virtual int write(const char* data, size_t len);
    virtual void flush();
};

#endif
