// Copyright (C) 2016 S117 <admin@0x10c.pw>
// Bitman Lab.
// Licensed under the terms of the GNU GPL, version 2
// http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt


#include <unistd.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <pthread.h>
#include <stdlib.h>
#include <assert.h>
#include "PVSC_Parser.h"
#include "DivaController.h"
#include "DSC_Reader.h"
#include "DS4_Output_STDOUT.h"
#include "DS4_Output_Serial.h"
#include "DSC_OperateEmitter.h"

static void inline timestamp() {
    timeval tv;
    gettimeofday(&tv,NULL);
    //printf("\n[Time] s:%ld, us:%d\n", tv.tv_sec, tv.tv_usec);
}

static inline uint32_t convert_time_to_ms(uint32_t dsc_time) {
    dsc_time = dsc_time/10;
    if(dsc_time%10 >= 5) {
        return dsc_time/10+1;
    } else {
        return dsc_time/10;
    }
}

void DivaController::reset_ds4_btn(DS4_Controller* controller) {
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

int64_t DivaController::get_first_note_time(DSC_Info *info) {
    std::list<TimeStamp*>::iterator it_ts = info->event_seq.begin();
    std::list<Inst*>::iterator it_inst;
    Note* cur_note;
    while(it_ts != info->event_seq.end()) {
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

static void post_sem(void* arg) {
    sem_post((sem_t*) arg);
}

int DivaController::work(diva_control_param& param) {
    const char* dsc_path = this->generate_dsc_path(param);
    sem_t* sem_is_finished;

    static const char* sem_name = "/divactlisfinished";
    sem_unlink(sem_name);
    if ((sem_is_finished = sem_open(sem_name, O_CREAT, 0644, 0)) == SEM_FAILED ) {
        perror("sem_open");
    }

    DSC_Info *info = DSC_Reader::read_dsc(dsc_path);
    if(info == nullptr) {
        fprintf(stderr, "Read DSC fail, check file \"%s\".", dsc_path);
        return -1;
    }

    DS4_Output_STDOUT output_stdout = DS4_Output_STDOUT();
    DS4_Output_Serial output_serial = DS4_Output_Serial();

    DS4_Operate op;

    DS4_Controller controller = DS4_Controller();
    controller.add_output(&output_serial);
    controller.add_output(&output_stdout);
    reset_ds4_btn(&controller);

    DSC_OperateEmitter dsc_emitter = DSC_OperateEmitter();
    printf("\nconverting\n");
    int64_t last_note_timestamp = dsc_emitter.emit(info, &controller, param.time_offset, param.is_add_salt);
    if(last_note_timestamp < 0) {
        fprintf(stderr, "Convert DSC file \"%s\" fail.\n", dsc_path);
        return -1;
    }

    op.key = op.PS;
    op.val = 0;
    op.cb = post_sem;
    op.cb_param = (void*)sem_is_finished;
    op.time_left_ms = (uint32_t)last_note_timestamp + 1000;
    controller.insert_operate(op); // use to notify the work is done

    printf("\nconverted.\n");
    printf("\ntimestamp_first_note:%llu\n", get_first_note_time(info));
    printf("\ntimestamp_last_note:%llu\n", last_note_timestamp);
    getchar();

    op.key = op.PS;
    op.val = 1;
    op.cb = NULL;
    op.time_left_ms = 0;
    controller.insert_operate(op);

    usleep(1000*1000);

    op.key = op.PS;
    op.val = 0;
    op.cb = NULL;
    op.time_left_ms = 0;
    controller.insert_operate(op); // use to power up the DS4

    usleep(1000*500); // wait some time

    op.key = op.BTN_CIRCLE;
    op.val = 1;
    op.cb = NULL;
    op.time_left_ms = 0;
    controller.insert_operate(op);

    usleep(1000*500);

    op.key = op.BTN_CIRCLE;
    op.val = 0;
    op.cb = NULL;
    op.time_left_ms = 0;
    controller.insert_operate(op); // use to start the game

    usleep(1000*500);

    printf("\nstart.\n");
    timestamp();
    delete info;
    controller.start_timer();

    int rtnval, err;
    do {
        rtnval = sem_wait(sem_is_finished);
        err = errno;
    } while((rtnval == -1) && (err = EINTR));

    sem_unlink(sem_name);
    sem_close(sem_is_finished);

    printf("\nfinished.\n");

    return 0;
}

const char* DivaController::generate_dsc_path(diva_control_param& param) {
    const char* path_pattern = "%s/.DivaController/PJD_%s/dscs/pv_%d_%s.dsc";
    const char* tbl_version[3] = {"F", "F2", "X"};
    const char* tbl_difficulty[4] = {"easy", "normal", "hard", "extreme"};
    assert(((0 <= param.version)&&( param.version <= 2)));
    assert(((0 <= param.difficulty)&&( param.difficulty <= 3)));
    sprintf(buffer_dsc_path, path_pattern, getenv("HOME"), tbl_version[param.version], param.pv_no,tbl_difficulty[param.difficulty]);
    return buffer_dsc_path;
}
