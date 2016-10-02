#include "DS4_Controller.h"
#include "stdio.h"
#include "errno.h"
#include <assert.h>
#include <sys/time.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <pthread.h>
#include <stdlib.h>


#define convert_timeval_to_us(tv) ((tv).tv_sec*1000000 + (tv).tv_usec)

static inline uint64_t convert_us_to_ms(uint64_t us){
    if((us/100)%10 > 4){
        return us/1000+1;
    }else{
        return us/1000;
    }
}

static const char* const TBL_CTRL_CODE_PATTERN[] = {
    "A%03hhu",
    "B%03hhu",
    "C%03hhu",
    "D%03hhu",
    "E%03hhu",
    "F%03hhu",
    "G%03hhu",
    "H%03hhu",
    "I%03hhu",
    "J%03hhu",
    "K%03hhu",
    "L%03hhu",
    "M%03hhu",
    "N%03hhu",
    "O%03hhu",
    "P%03hhu",
    "Q%03hhu",
    "R%03hhu",
    "S%03hhu",
    "Z%03hhu",
};
static const int LEN_CTRL_CODE = 5;

static void timer_trampoline(void *args) {
    return DS4_Controller::tick((DS4_Controller*)args);
}

static void* dispatcher_trampoline(void *args) {
    return DS4_Controller::dispatch_check((DS4_Controller*) args);
}

static void move_pthread_to_realtime_scheduling_class(pthread_t pthread)
{
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

DS4_Controller::DS4_Controller(){
    INIT_LIST_HEAD(&m_output_list);
    
    INIT_LIST_HEAD(&m_op_pend_FIFO);
    pthread_mutex_init(&m_mtx_pend_FIFO, NULL);
    
    INIT_LIST_HEAD(&m_op_ready_FIFO);
    pthread_mutex_init(&m_mtx_ready_FIFO, NULL);
    
    pthread_mutex_init(&m_mtx_tick, NULL);
    //sem_init(&m_sem_ready, 0, 0);
    sem_unlink("/dscsemardy");
    if ((m_sem_ready = sem_open("/dscsemardy", O_CREAT, 0644, 0)) == SEM_FAILED ) {
        perror("sem_open");
    }

    m_gt = GlobalTimer1ms::get_instance();
    m_routine_id = m_gt->add_routine(timer_trampoline, this);
    m_isRunning = true;
    pthread_create(&m_dispatcher_thread_id, NULL, dispatcher_trampoline, this);
    move_pthread_to_realtime_scheduling_class(m_dispatcher_thread_id);
}

DS4_Controller::~DS4_Controller(){
    m_gt->stop_timer();
    m_gt->del_routine(m_routine_id);
    m_isRunning = false;
    sem_post(m_sem_ready);
    pthread_join(m_dispatcher_thread_id, NULL);
    sem_unlink("/dscsemardy");
    sem_close(m_sem_ready);
}

int DS4_Controller::start_timer(){
    timeval start_time;
    gettimeofday(&start_time, nullptr);
    m_timestamp_start = convert_timeval_to_us(start_time);
    m_gt->start_timer();
    return 0;
}

void DS4_Controller::insert_operate(DS4_Operate& op){
#ifdef DEBUG
    if((op.key < op.STICK_L_X_ANALOG) &&
    ((op.val != 0) && (op.val != 1))){
        fprintf(stderr, "invalid key value, key: %d, val:%d\n", op.key, op.val);
        return;
    }
#endif
    DS4_Operate *new_op = new DS4_Operate;
    *new_op = op;
    if(new_op->time_left_ms == 0){
        pthread_mutex_lock(&m_mtx_ready_FIFO);
        list_add(&new_op->list, &m_op_ready_FIFO);
        pthread_mutex_unlock(&m_mtx_ready_FIFO);
        sem_post(m_sem_ready);
    }else{
        pthread_mutex_lock(&m_mtx_pend_FIFO);
        DS4_Operate *cursor;
        list_for_each_entry(cursor, &m_op_pend_FIFO, list){ // find the insert position in time differential chain
            if(cursor->time_left_ms > new_op->time_left_ms){
                break;
            }
        }
        list_add_tail(&new_op->list, &cursor->list);
        pthread_mutex_unlock(&m_mtx_pend_FIFO);
    }
}

#ifdef DEBUG
volatile int tick_overlap_flag;
#endif

// internal method for timer's trampoline function, DO NOT call this method explicitly
void DS4_Controller::tick(DS4_Controller* ctrl){
#ifdef DEBUG
    if(tick_overlap_flag){
        printf("TICK OVERLAPED!, %d\n", tick_overlap_flag);
    }
    ++tick_overlap_flag;
#endif
    
    timeval tv;
    //uint64_t timestamp_now;
    uint64_t time_elapsed;
    pthread_mutex_lock(&ctrl->m_mtx_pend_FIFO);

    DS4_Operate *cursor, *n;
    gettimeofday(&tv, nullptr);
    time_elapsed = convert_timeval_to_us(tv) - ctrl->m_timestamp_start;
    
    //pthread_mutex_lock(&ctrl->m_mtx_tick);
    list_for_each_entry_safe(cursor, n, &ctrl->m_op_pend_FIFO, list){
        if(((int64_t)(cursor->time_left_ms - convert_us_to_ms(time_elapsed)) > 0)){
            break;
        }
            
        pthread_mutex_lock(&ctrl->m_mtx_ready_FIFO);
        list_move_tail(&cursor->list, &ctrl->m_op_ready_FIFO);
        pthread_mutex_unlock(&ctrl->m_mtx_ready_FIFO);
        sem_post(ctrl->m_sem_ready);
        //puts("S!\n");
    }
    //pthread_mutex_unlock(&ctrl->m_mtx_tick);
    pthread_mutex_unlock(&ctrl->m_mtx_pend_FIFO);
#ifdef DEBUG
    --tick_overlap_flag;
#endif
    return;
}

void* DS4_Controller::dispatch_check(DS4_Controller *ctrl){
    char buffer[LEN_CTRL_CODE+1];
    DS4_Operate* op;
    DS4_Output_List *cursor;
    int rtnval, err;
    //std::list<DS4_Output*>::iterator it;
    while(ctrl->isRunning()){
        do{
            rtnval = sem_wait(ctrl->m_sem_ready);
            err = errno;
        }while((rtnval == -1) && (err = EINTR));
    
        if(!ctrl->isRunning())
            return 0;
        pthread_mutex_lock(&ctrl->m_mtx_ready_FIFO);
        //printf("m_op_ready_FIFO.size()=%lu\n", ctrl->m_op_ready_FIFO.size());
        assert(!list_empty(&ctrl->m_op_ready_FIFO));
        op = list_entry(ctrl->m_op_ready_FIFO.next, DS4_Operate, list);
        list_del(&op->list);
        
        //op = ctrl->m_op_ready_FIFO.next;
        //printf("m_op_ready_FIFO.size()=%lu\n", ctrl->m_op_ready_FIFO.size());
        pthread_mutex_unlock(&ctrl->m_mtx_ready_FIFO);
        sprintf(buffer, TBL_CTRL_CODE_PATTERN[op->key], op->val);
        
        list_for_each_entry(cursor, &ctrl->m_output_list, list){
            cursor->output->write(buffer, 4);
            cursor->output->flush();
        }
        if(op->cb) op->cb(*op);
        delete op;
//        it = ctrl->m_output.begin();
//        while(it != ctrl->m_output.end()){
//            (*it)->write(buffer);
//            (*it)->flush();
//            ++it;
//        }
    }
    return 0;
}

void DS4_Controller::add_output(DS4_Output* new_output){
    if(new_output == nullptr)
        return;
    DS4_Output_List *new_node = new DS4_Output_List;
    new_node->output = new_output;
    list_add(&new_node->list, &m_output_list);
}

bool DS4_Controller::isRunning(){
    return m_isRunning;
}

