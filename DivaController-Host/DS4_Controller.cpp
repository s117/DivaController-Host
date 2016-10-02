#include "DS4_Controller.h"
#include "stdio.h"
#include "errno.h"
#include <assert.h>

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

DS4_Controller::DS4_Controller(GlobalTimer1ms* gt){
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

    m_gt = gt;
    m_routine_id = gt->add_routine(timer_trampoline, this);
    m_isRunning = true;
    pthread_create(&m_dispatcher_thread_id, NULL, dispatcher_trampoline, this);
}

DS4_Controller::~DS4_Controller(){
    m_gt->stop_timer();
    m_gt->del_routine(m_routine_id);
    m_isRunning = false;
    sem_post(m_sem_ready);
    //puts("S!\n");
    pthread_join(m_dispatcher_thread_id, NULL);
    sem_unlink("/dscsemardy");
    sem_close(m_sem_ready);
}

void DS4_Controller::insert_operate(DS4_Operate& op){
    if((op.key < op.STICK_L_X_ANALOG) &&
    ((op.val != 0) && (op.val != 1))){
#ifdef DEBUG
        fprintf(stderr, "invalid key value, key: %d, val:%d\n", op.key, op.val);
#endif
        return;
    }
    DS4_Operate *new_op = new DS4_Operate;
    *new_op = op;
    if(new_op->time_left_ms == 0){
        pthread_mutex_lock(&m_mtx_ready_FIFO);
        list_add(&new_op->list, &m_op_ready_FIFO);
        //m_op_ready_FIFO.push_back(op);
        pthread_mutex_unlock(&m_mtx_ready_FIFO);
        sem_post(m_sem_ready);
        //puts("S!\n");
    }else{
        pthread_mutex_lock(&m_mtx_pend_FIFO);
        //std::list<DS4_Operate>::iterator it =  m_op_pend_FIFO.begin();
        DS4_Operate *cursor;
        int delay_sig_prev = 0, delay_sig_curr = 0;
        //while(it != m_op_pend_FIFO.end()){ // find the insert position in time differential chain
        list_for_each_entry(cursor, &m_op_pend_FIFO, list){
            if(cursor->time_left_ms!=0){
                delay_sig_prev = delay_sig_curr;
                delay_sig_curr += cursor->time_left_ms;
                if(delay_sig_curr > new_op->time_left_ms){
                    break;
                }
            }
        }
        if(&cursor->list == (&m_op_pend_FIFO)){ // insert at end
            new_op->time_left_ms -= delay_sig_curr;
            //m_op_pend_FIFO.push_back(op);
            list_add_tail(&new_op->list, &m_op_pend_FIFO);
        }else{ // insert in the middle
            new_op->time_left_ms -= delay_sig_prev;
            //m_op_pend_FIFO.insert(it, op);
            list_add_tail(&new_op->list, &cursor->list);
            cursor->time_left_ms -= new_op->time_left_ms;
        }
        //m_op_pend_FIFO.push_back(op);
        pthread_mutex_unlock(&m_mtx_pend_FIFO);
    }
}
volatile int overlap_flag;
// internal method for timer's trampoline function, DO NOT call this method explicitly
void DS4_Controller::tick(DS4_Controller* ctrl){
    if(overlap_flag){
        printf("TICK OVERLAPED!, %d\n", overlap_flag);
    }
    ++overlap_flag;
    pthread_mutex_lock(&ctrl->m_mtx_pend_FIFO);
    //if(ctrl->m_op_pend_FIFO.size() == 0){
    if(list_empty(&ctrl->m_op_pend_FIFO)){
        pthread_mutex_unlock(&ctrl->m_mtx_pend_FIFO);
        --overlap_flag;
        return;
    }
    //std::list<DS4_Operate>::iterator it = ctrl->m_op_pend_FIFO.begin();
    DS4_Operate *cursor, *n;
    cursor = list_entry(ctrl->m_op_pend_FIFO.next, DS4_Operate, list);
    //pthread_mutex_lock(&ctrl->m_mtx_tick);
    --(cursor->time_left_ms);
    list_for_each_entry_safe(cursor, n, &ctrl->m_op_pend_FIFO, list){
        if(cursor->time_left_ms != 0)
            break;
        pthread_mutex_lock(&ctrl->m_mtx_ready_FIFO);
        list_move_tail(&cursor->list, &ctrl->m_op_ready_FIFO);
        //ctrl->m_op_ready_FIFO.push_back(*it);
        //it = ctrl->m_op_pend_FIFO.erase(it);
        pthread_mutex_unlock(&ctrl->m_mtx_ready_FIFO);
        sem_post(ctrl->m_sem_ready);
        //puts("S!\n");
    }
    //pthread_mutex_unlock(&ctrl->m_mtx_tick);
    pthread_mutex_unlock(&ctrl->m_mtx_pend_FIFO);
    --overlap_flag;
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

