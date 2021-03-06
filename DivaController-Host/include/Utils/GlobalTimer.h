// Copyright (C) 2016 S117 <admin@0x10c.pw>
// Bitman Lab.
// Licensed under the terms of the GNU GPL, version 2
// http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt


#ifndef __H_GLOBAL_TIMER
#define __H_GLOBAL_TIMER

#include <stdio.h>
#include <list>
#include <pthread.h>
#include <sched.h>
#include <semaphore.h>
#include <sys/time.h>
#include "list.h"

typedef void (*timer_routine)(void* data);
typedef size_t gtimer_t;

typedef struct __routine_record {
    struct list_head list;

    timer_routine entry;
    void* data;
    gtimer_t id;
} routine_record_t;

class GlobalTimer1ms {
public:
    // DO NOT add a routine when the timer is running
    gtimer_t add_routine(timer_routine entry, void* data);
    void del_routine(gtimer_t id);
    bool isrunning();
    void start_timer();
    void stop_timer();
    static GlobalTimer1ms* get_instance();
    static void tick();
private:
    GlobalTimer1ms();
    ~GlobalTimer1ms();
    gtimer_t id_alloc;
    static GlobalTimer1ms* global_instance;

    bool running;
    struct itimerval itimer_val;
    pthread_mutex_t mutex_routine_list;
    struct list_head routine_list;
    sigset_t old_main_set;
    pthread_t tick_thread;
};

#endif /* GlobalTimer_hpp */
