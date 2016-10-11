// Copyright (C) 2016 S117 <admin@0x10c.pw>
// Bitman Lab.
// Licensed under the terms of the GNU GPL, version 2
// http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt


#include "GlobalTimer.h"
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <mach/mach.h>
#include <mach/mach_time.h>

static void handle_error_en(int i, const char* info) {
    fprintf(stderr, "[%p] Error: %s return %d\n", pthread_self(), info, i);
    exit(-1);
}

static void sig_routine(int dunno) {
    switch (dunno) {
    case SIGALRM:
        GlobalTimer1ms::tick();
        break;
    }
    return;
}

static void* tick_thread_routine(void* arg) {
    sigset_t *set = (sigset_t*)arg;
    int s;
    s = pthread_sigmask(SIG_UNBLOCK, set, NULL);
    if (s != 0)
        handle_error_en(s, "pthread_sigmask");
    for(;;) {
        pthread_testcancel();
        pause();
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

    // change the way how thread handle the SIGALRM
    int s;
    struct sigaction sigact;
    sigemptyset( &sigact.sa_mask );
    sigact.sa_flags = 0;
    sigact.sa_handler = sig_routine;
    sigaction( SIGALRM, &sigact, NULL );

    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGALRM);
    s = pthread_sigmask(SIG_BLOCK, &set, &old_main_set); // BLOCK the SIGALRM for main thread
    if (s != 0)
        handle_error_en(s, "pthread_sigmask");

    s = pthread_create(&tick_thread, NULL, tick_thread_routine, (void *) &set); // create a dedicated thread for SIGALRM
    if (s != 0)
        handle_error_en(s, "pthread_create");

    move_pthread_to_realtime_scheduling_class(pthread_self()); // move SIGALRM handle thread to realtime scheduling class
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

    pthread_cancel(tick_thread); // cancel the SIGALRM dedicated thread
    pthread_join(tick_thread, NULL); // wait thread terminated
    pthread_sigmask(SIG_SETMASK, &old_main_set, NULL); // recover main thread's old signal mask
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
}
