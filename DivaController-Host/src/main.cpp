//
//  main.cpp
//  DivaController-Host
//
//  Created by s117 on 16/9/30.
//  Copyright © 2016年 s117. All rights reserved.
//
#include "PVSC_Parser.h"
#include "DS4_Controller.h"
#include "DSC_Reader.h"
#include "DS4_Output_STDOUT.h"
#include "DS4_Output_Serial.h"
#include "DSC_OperateEmitter.h"
#include <unistd.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <pthread.h>
#include <stdlib.h>


void inline timestamp() {
    timeval tv;
    gettimeofday(&tv,NULL);
    printf("[Time] s:%ld, us:%d\n", tv.tv_sec, tv.tv_usec);
}

void opcb(DS4_Operate& op) {
    timestamp();
}

void reset_ds4_btn(DS4_Controller* controller) {
    DS4_Operate op;
    uint8_t stick_mid = 127;
    for(int i = DS4_Operate::PS; i < DS4_Operate::STICK_L_X_ANALOG; ++i) {
        op.key = (DS4_Operate::KEY_CODE)i;
        op.val = 0;
        op.cb = NULL;
        op.time_left_ms = 0;
        controller->insert_operate(op);
        usleep(10000);
    }
    for(int i = DS4_Operate::STICK_L_X_ANALOG; i < DS4_Operate::RESERVED; ++i) {
        op.key = (DS4_Operate::KEY_CODE)i;
        op.val = stick_mid;
        op.cb = NULL;
        op.time_left_ms = 0;
        controller->insert_operate(op);
        usleep(10000);
    }
}

static void move_pthread_to_realtime_scheduling_class(pthread_t pthread) {
    mach_timebase_info_data_t timebase_info;
    mach_timebase_info(&timebase_info);

    const uint64_t NANOS_PER_MSEC = 1000000ULL;
    double clock2abs = ((double)timebase_info.denom / (double)timebase_info.numer) * NANOS_PER_MSEC;

    thread_time_constraint_policy_data_t policy;
    policy.period      = 0;
    policy.computation = (uint32_t)(5 * clock2abs); // 5 ms of work
    policy.constraint  = (uint32_t)(10 * clock2abs);
    policy.preemptible = FALSE;

    int kr = thread_policy_set(pthread_mach_thread_np(pthread_self()),
                               THREAD_TIME_CONSTRAINT_POLICY,
                               (thread_policy_t)&policy,
                               THREAD_TIME_CONSTRAINT_POLICY_COUNT);
    if (kr != KERN_SUCCESS) {
        mach_error("thread_policy_set:", kr);
        exit(1);
    }
}

static inline uint32_t convert_time_to_ms(uint32_t dsc_time) {
    dsc_time = dsc_time/10;
    if(dsc_time%10 >= 5) {
        return dsc_time/10+1;
    } else {
        return dsc_time/10;
    }
}

int64_t get_first_note_time(DSC_Info *info) {
    std::list<TimeStamp*>::iterator it_ts = info->event_seq.begin();
    std::list<Inst*>::iterator it_inst;
    Note* cur_note;
    while(it_ts != info->event_seq.end()) {
        TimeStamp* ts = *it_ts;
        for(it_inst = (*it_ts)->inst_list.begin();
                it_inst != (*it_ts)->inst_list.end();
                ++it_inst) {
            if((*it_inst)->itype == Inst::InstType::NOTE) {
                cur_note = (Note*)((*it_inst)->idata);
                return convert_time_to_ms(cur_note->note_time_offset) + convert_time_to_ms((*it_ts)->time);
            }
        }
        ++it_ts;
    }
    return 0;
}

struct _task_descriptor {

} task_descriptor_t;

int main(int argc, char* argv[]) {
    //DSC_Info *info = DSC_Reader::read_dsc("/Users/s117/Bitman/psv/PDJ_dsc_X/pv_824_extreme.dsc");
    //DSC_Info *info = DSC_Reader::read_dsc("/Users/s117/Bitman/psv/PDJ_dsc_X/pv_830_extreme.dsc");
    DSC_Info *info = DSC_Reader::read_dsc("/Users/s117/Bitman/psv/PDJ_dsc_X/pv_816_extreme.dsc");
    if(info == nullptr) {
        perror("Read DSC fail, terminated ");
        exit(-1);
    }

    move_pthread_to_realtime_scheduling_class(pthread_self());

    DS4_Output_STDOUT *output_stdout = new DS4_Output_STDOUT();
    DS4_Output_Serial *output_serial = new DS4_Output_Serial();

    GlobalTimer1ms *gt = GlobalTimer1ms::get_instance();
    DS4_Operate op;

    DS4_Controller *controller = new DS4_Controller();
    controller->add_output(output_serial);
    controller->add_output(output_stdout);
    reset_ds4_btn(controller);

    DSC_OperateEmitter dsc_emitter = DSC_OperateEmitter();
    printf("converting\n");
    //if(dsc_emitter.emit(info, controller, -1090, false)){ // ninjia_easy
    //if(dsc_emitter.emit(info, controller, -1440, false)){ // satisfication_ex
    //if(dsc_emitter.emit(info, controller, -560, false)){ // ultimate_ex

    if(dsc_emitter.emit(info, controller, -850, false)) {
        perror("Convert DSC fail.\n");
        exit(-1);
    }

    printf("converted.\n");
    uint64_t t = get_first_note_time(info);
    printf("first_note:%llu\n", t);
    getchar();


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

    usleep(100);
    printf("press enter to calibrate.\n");
    //getchar();
    printf("start.\n");
    timestamp();

    controller->start_timer();

    for(;;) {
        sleep(1);
    }

    delete info;
    return 0;
}


