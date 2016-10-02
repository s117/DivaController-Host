//
//  DSC_Reader.hpp
//  DivaController-Host
//
//  Created by s117 on 16/9/30.
//  Copyright © 2016年 s117. All rights reserved.
//

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

typedef struct __DS4_Operate{
    struct list_head list;
    typedef void(*cb_operate_done_t)(__DS4_Operate& op);
    
    enum KEY_CODE{
        PS = 0,
        BTN_TRIANGLE,
        BTN_CIRCLE,
        BTN_CROSS,
        BTN_SQUARE,
        SHLD_L1,
        SHLD_R1,
        DPAD_UP,
        DPAD_RIGHT,
        DPAD_DOWN,
        DPAD_LEFT,
        STICK_LS,
        STICK_RS,
        STICK_L_X_ANALOG,
        STICK_L_Y_ANALOG,
        STICK_R_X_ANALOG,
        STICK_R_Y_ANALOG,
        TRIGR_LT_ANALOG,
        TRIGR_RT_ANALOG,
        RESERVED,
    };

    KEY_CODE key;
    uint8_t val;
    
    // when use as argument, this field use to indicate the delay of instruction emit,
    // when in pend queue, this field use as time differential chain
    uint32_t time_left_ms;
    cb_operate_done_t cb;
} DS4_Operate;

typedef struct __DS4_Output_List{
    struct list_head list;
    DS4_Output* output;
} DS4_Output_List;

class DS4_Controller{
public:
    DS4_Controller(GlobalTimer1ms *gt);
    ~DS4_Controller();
    void insert_operate(DS4_Operate& op);
    void add_output(DS4_Output* new_output);
    //void del_output;
    
    // following method is degsigned for trampoline function, DO NOT call this method explicitly
    static void tick(DS4_Controller* ctrl);
    static void* dispatch_check(DS4_Controller* ctrl);
    bool isRunning();
private:
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
