// Copyright (C) 2016 S117 <admin@0x10c.pw>
// Bitman Lab.
// Licensed under the terms of the GNU GPL, version 2
// http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt


#include "DSC_Reader.h"
#include "utils.h"
#include "stdlib.h"

DSC_Info* DSC_Reader::read_dsc(const char* path) {
    size_t file_len;
    uint8_t* file = get_file_content(path, &file_len);
    if(file == NULL) {
        return NULL;
    }
    PVSC_Parser* parser = new PVSC_Parser();
    DSC_Info *info = new DSC_Info;
    STEP rtnval = parser->parse_dsc(info, file, (uint32_t)file_len);
    if(rtnval != STEP_END_OF_SEQUENCE) {
        free(file);
        delete parser;
        delete info;
        return nullptr;
    }
    free(file);
    delete parser;
    return info;
}
