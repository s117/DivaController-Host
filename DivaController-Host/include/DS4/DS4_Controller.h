// Copyright (C) 2016 S117 <admin@0x10c.pw>
// Bitman Lab.
// Licensed under the terms of the GNU GPL, version 2
// http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt


#ifndef __H_DS4_CONTROLLER
#define __H_DS4_CONTROLLER

#include <stdio.h>
#include "DSC_Info.h"
#include "DS4_Output.h"
#include <pthread.h>
#include <sched.h>
#include <semaphore.h>
#include <signal.h>
#include <time.h>
#include <vector>
#include <list>
#include "GlobalTimer.h"

typedef struct __DS4_Operate {
    struct list_head list;
    typedef void(*cb_operate_done_t)(void* param);

    enum KEY_CODE {
        PS = 0,
        BTN_TRIANGLE = 1,
        BTN_CIRCLE = 2,
        BTN_CROSS = 3,
        BTN_SQUARE = 4,
        SHLD_L1 = 5,
        SHLD_R1 = 6,
        DPAD_UP = 7,
        DPAD_RIGHT = 8,
        DPAD_DOWN = 9,
        DPAD_LEFT = 10,
        STICK_LS = 11,
        STICK_RS = 12,
        STICK_L_X_ANALOG = 13,
        STICK_L_Y_ANALOG = 14,
        STICK_R_X_ANALOG = 15,
        STICK_R_Y_ANALOG = 16,
        TRIGR_LT_ANALOG = 17,
        TRIGR_RT_ANALOG = 18,
        RESERVED = 19,
    };

    KEY_CODE key;
    uint8_t val;

    // when use as argument, this field use to indicate the delay of instruction emit,
    // when in pend queue, this field use as time differential chain
    uint32_t time_left_ms;
    void* cb_param;
    cb_operate_done_t cb;
} DS4_Operate;

typedef struct __DS4_Output_List {
    struct list_head list;
    DS4_Output* output;
} DS4_Output_List;

class DS4_Controller {
public:
    DS4_Controller();
    ~DS4_Controller();
    void insert_operate(DS4_Operate& op);
    void add_output(DS4_Output* new_output);
    int start_timer();

    // following method is degsigned for trampoline function, DO NOT call this method explicitly
    static void tick(DS4_Controller* ctrl);
    static void* dispatch_check(DS4_Controller* ctrl);
    bool isRunning();
private:
    uint64_t m_timestamp_start;
    pthread_t m_dispatcher_thread_id;
    bool m_isRunning;
    GlobalTimer1ms *m_gt;
    gtimer_t m_routine_id;

    struct list_head m_op_pend_FIFO;
    pthread_mutex_t m_mtx_pend_FIFO;

    struct list_head m_op_ready_FIFO;
    sem_t* m_sem_ready;
    pthread_mutex_t m_mtx_ready_FIFO;

    pthread_mutex_t m_mtx_tick;

    struct list_head m_output_list;
};
#endif /* DSC_Reader_hpp */
