//
//  DSC_Reader.cpp
//  DivaController-Host
//
//  Created by s117 on 16/9/30.
//  Copyright © 2016年 s117. All rights reserved.
//

#include "DSC_Reader.h"
#include "utils.h"
#include "stdlib.h"

DSC_Info* DSC_Reader::read_dsc(const char* path){
    size_t file_len;
    uint8_t* file = get_file_content(path, &file_len);
    if(file == NULL){
        return NULL;
    }
    PVSC_Parser* parser = new PVSC_Parser();
    DSC_Info *info = new DSC_Info;
    STEP rtnval = parser->parse_dsc(info, file, (uint32_t)file_len);
    if(rtnval != STEP_END_OF_SEQUENCE){
        free(file);
        delete parser;
        delete info;
        return nullptr;
    }
    free(file);
    delete parser;
    return info;
}
