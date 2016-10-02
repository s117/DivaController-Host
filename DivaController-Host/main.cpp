//
//  main.cpp
//  DivaController-Host
//
//  Created by s117 on 16/9/30.
//  Copyright © 2016年 s117. All rights reserved.
//
#include "PVSC_Parser.h"
#include "pv_801_extreme.h"
#include "DS4_Controller.h"
#include "DSC_Reader.h"
#include "DS4_Output_STDOUT.h"
#include "DS4_Output_Serial.h"
#include "DSC_OperateEmitter.h"
#include <unistd.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>

static timeval tv;

void inline timestamp(){
    gettimeofday(&tv,NULL);
    printf("[Time] s:%d, us:%d\n", tv.tv_sec, tv.tv_usec);
}

void opcb(DS4_Operate& op){
    timestamp();
}

void reset_ds4_btn(DS4_Controller* controller){
    DS4_Operate op;
    uint8_t stick_mid = 127;
    for(int i = DS4_Operate::PS; i < DS4_Operate::STICK_L_X_ANALOG; ++i){
        op.key = (DS4_Operate::KEY_CODE)i;
        op.val = 0;
        op.cb = NULL;
        op.time_left_ms = 0;
        controller->insert_operate(op);
        usleep(10000);
    }
    for(int i = DS4_Operate::STICK_L_X_ANALOG; i < DS4_Operate::RESERVED; ++i){
        op.key = (DS4_Operate::KEY_CODE)i;
        op.val = stick_mid;
        op.cb = NULL;
        op.time_left_ms = 0;
        controller->insert_operate(op);
        usleep(10000);
    }
}

int main(int argc, char* argv[]) {
    DSC_Info *info = DSC_Reader::read_dsc("/Users/s117/Bitman/psv/PDJ_dsc_X/pv_813_easy.dsc");
    if(info == nullptr){
        perror("Read DSC fail, terminated ");
        exit(-1);
    }
    
    DS4_Output_STDOUT *output_stdout = new DS4_Output_STDOUT();
    DS4_Output_Serial *output_serial = new DS4_Output_Serial();
    
    GlobalTimer1ms *gt = GlobalTimer1ms::get_instance();
    DS4_Operate op;
    
    DS4_Controller *controller = new DS4_Controller(GlobalTimer1ms::get_instance());
    controller->add_output(output_serial);
    controller->add_output(output_stdout);
    reset_ds4_btn(controller);
    
    DSC_OperateEmitter dsc_emitter = DSC_OperateEmitter();
    printf("converting\n");
    if(dsc_emitter.emit(info, controller, -1090, false)){
        perror("Convert DSC fail.\n");
        exit(-1);
    }
    printf("converted.\n");

    getchar();
    printf("start.\n");
    
    op.key = op.BTN_CIRCLE;
    op.val = 1;
    op.cb = NULL;
    op.time_left_ms = 0;
    controller->insert_operate(op);
    
    op.key = op.BTN_CIRCLE;
    op.val = 0;
    op.cb = NULL;
    op.time_left_ms = 50;
    controller->insert_operate(op);
    
    
    
    timestamp();
    
    delete info;
    
    gt->start_timer();
    
    for(;;){
        sleep(1);
    }
    
    return 0;
}


