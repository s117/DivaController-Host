//
//  main.cpp
//  DivaController-Host
//
//  Created by s117 on 16/9/30.
//  Copyright © 2016年 s117. All rights reserved.
//
#include "PVSC_Parser.h"
#include "pv_801_extreme.h"

int main(int argc, char* argv[]) {
    DSC_Info info;
    PVSC_Parser *parser = new PVSC_Parser();
    parser->parse_dsc(&info, data, sizeof(data));
    
    return 0;
}
