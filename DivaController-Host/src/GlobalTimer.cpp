//
//  GlobalTimer.cpp
//  timer_test
//
//  Created by s117 on 16/10/1.
//  Copyright © 2016年 s117. All rights reserved.
//

#include "GlobalTimer.h"


#include <signal.h>

static void sigroutine(int dunno) {
    switch (dunno) {
    case SIGALRM:
        GlobalTimer1ms::tick();
        break;
    }
    return;
}

#define INVERTAL_S  0
#define INVERTAL_US 1000
GlobalTimer1ms* GlobalTimer1ms::global_instance = nullptr;

GlobalTimer1ms::GlobalTimer1ms() {
    running = false;
    id_alloc = 0;
    INIT_LIST_HEAD(&routine_list);
    pthread_mutex_init(&mutex_routine_list, NULL);
    this->itimer_val.it_value.tv_sec = INVERTAL_S;
    this->itimer_val.it_value.tv_usec = INVERTAL_US;
    this->itimer_val.it_interval.tv_sec = INVERTAL_S;
    this->itimer_val.it_interval.tv_usec = INVERTAL_US;
}


GlobalTimer1ms::~GlobalTimer1ms() {
    global_instance = nullptr;
    routine_record_t *cursor, *n;
    list_for_each_entry_safe(cursor, n, &routine_list, list) {
        delete cursor;
    }

    this->itimer_val.it_value.tv_sec = 0;
    this->itimer_val.it_value.tv_usec = 0;
    this->itimer_val.it_interval.tv_sec = 0;
    this->itimer_val.it_interval.tv_usec = 0;
    setitimer(ITIMER_REAL, &itimer_val, NULL);
}

GlobalTimer1ms* GlobalTimer1ms::get_instance() {
    if(global_instance == nullptr)
        global_instance = new GlobalTimer1ms();
    return global_instance;
}

gtimer_t GlobalTimer1ms::add_routine(timer_routine entry, void* data) {
    if(entry == nullptr)
        return -1;
    routine_record_t *routine = new routine_record_t;
    routine->entry = entry;
    routine->data = data;
    routine->id = ++id_alloc;

    pthread_mutex_lock(&mutex_routine_list);
    //id = routine_list.size();
    //routine_list.push_back(routine);
    list_add_tail(&routine->list, &routine_list);
    pthread_mutex_unlock(&mutex_routine_list);

    return routine->id;
}
void GlobalTimer1ms::del_routine(gtimer_t id) {
    //std::list<routine_record>::iterator it = routine_list.begin();
    pthread_mutex_lock(&mutex_routine_list);
    routine_record_t *cursor, *n;
    list_for_each_entry_safe(cursor, n, &routine_list, list) {
        if(cursor->id == id) {
            delete cursor;
            break;
        }
    }
    pthread_mutex_unlock(&mutex_routine_list);
}

void GlobalTimer1ms::start_timer() {
    if(this->running == true) {
        return;
    }
    this->running = true;

    struct sigaction sigact;
    sigemptyset( &sigact.sa_mask );
    sigact.sa_flags = 0;
    sigact.sa_handler = sigroutine;
    sigaction( SIGALRM, &sigact, NULL );

    this->itimer_val.it_value.tv_sec = INVERTAL_S;
    this->itimer_val.it_value.tv_usec = INVERTAL_US;
    this->itimer_val.it_interval.tv_sec = INVERTAL_S;
    this->itimer_val.it_interval.tv_usec = INVERTAL_US;
    setitimer(ITIMER_REAL, &itimer_val, NULL);
}

void GlobalTimer1ms::stop_timer() {
    if(this->running == false) {
        return;
    }
    this->running = false;

    this->itimer_val.it_value.tv_sec = 0;
    this->itimer_val.it_value.tv_usec = 0;
    this->itimer_val.it_interval.tv_sec = 0;
    this->itimer_val.it_interval.tv_usec = 0;
    setitimer(ITIMER_REAL, &itimer_val, NULL);
}

bool GlobalTimer1ms::isrunning() {
    return running;
}

void GlobalTimer1ms::tick() {
    GlobalTimer1ms* gt = GlobalTimer1ms::get_instance();
    routine_record_t *cursor;
    list_for_each_entry(cursor, &gt->routine_list, list) {
        cursor->entry(cursor->data);
    }
//    for(std::list<routine_record>::iterator it = gt->routine_list.begin();it != gt->routine_list.end(); ++it){
//        it->entry(it->data);
//    }
}
