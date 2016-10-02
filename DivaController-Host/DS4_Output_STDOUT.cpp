//
//  DSC_Reader.cpp
//  DivaController-Host
//
//  Created by s117 on 16/9/30.
//  Copyright © 2016年 s117. All rights reserved.
//

#include "DS4_Output_STDOUT.h"
#include <stdio.h>

int DS4_Output_STDOUT::write(const char* data, size_t len){
    for(int i = 0;i<len;++i){
        fputc(data[i], stdout);
    }
    return 0;
}

void DS4_Output_STDOUT::flush(){
    fflush(stdout);
}


