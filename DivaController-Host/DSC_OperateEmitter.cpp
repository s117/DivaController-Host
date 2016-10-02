
#include <stdio.h>
#include "DSC_OperateEmitter.h"
#include "DSC_Info.h"
#include <assert.h>



#define BTN_SINGLE_PUSH_ADVANCE_TIME_MS         (0)
#define BTN_SINGLE_HOLD_TIME_MS                 (50)
#define BTN_SINGLE_EXCHANGE_THRES               (200)

#define BTN_DUAL_PUSH_ADVANCE_TIME_MS           BTN_SINGLE_PUSH_ADVANCE_TIME_MS
#define BTN_DUAL_HOLD_TIME_MS                   BTN_SINGLE_HOLD_TIME_MS

#define BTN_HOLD_ADVANCE_TIME_MS                (0)
#define BTN_HOLD_RELEASE_DELAY_TIME_MS          (0)

#define STICK_DAC_MID                           (127)
#define STICK_DAC_MIN                           (0)
#define STICK_DAC_MAX                           (255)

#define STAR_SLIDE_ADVANCE_TIME_MS              (0)
#define STAR_SLIDE_TRIGGE_POINT_DIV_N           (4)
#define STAR_SLIDE_WINDOW_MS                    (40)
#define STAR_SLIDE_STEP_N                       (8)

#define STAR_DUAL_SLIDE_ADVANCE_TIME_MS         STAR_SLIDE_ADVANCE_TIME_MS
#define STAR_DUAL_SLIDE_TRIGGE_POINT_DIV_N      STAR_SLIDE_TRIGGE_POINT_DIV_N
#define STAR_DUAL_SLIDE_WINDOW_MS               STAR_SLIDE_WINDOW_MS
#define STAR_DUAL_SLIDE_STEP_N                  STAR_SLIDE_STEP_N

#define RUSH_PUSH_ADVANCE_TIME_MS               (0)
#define RUSH_INVERVAL_MS                        (50)
#define RUSH_IS_DUAL                            (1)

#define STICK_TIME_MS    (100)
#define BTN_CONVERT_R2L(R)  ((DS4_Operate::KEY_CODE)((int)(R)+6))
#define BTN_CONVERT_L2R(L)  ((DS4_Operate::KEY_CODE)((int)(L)-6))
#define STIWCH_STICK(S)     ((S) == DS4_Operate::KEY_CODE::STICK_R_X_ANALOG ? \
                            DS4_Operate::KEY_CODE::STICK_L_X_ANALOG : DS4_Operate::KEY_CODE::STICK_R_X_ANALOG )
const DS4_Operate::KEY_CODE TBL_CONVERT[] = {
    DS4_Operate::KEY_CODE::BTN_TRIANGLE,     //TRIANGLE        = 0,
    DS4_Operate::KEY_CODE::BTN_CIRCLE,       //CIRCLE          = 1,
    DS4_Operate::KEY_CODE::BTN_CROSS,        //CROSS           = 2,
    DS4_Operate::KEY_CODE::BTN_SQUARE,       //SQUARE          = 3,
    DS4_Operate::KEY_CODE::DPAD_UP,          //UP              = 4,
    DS4_Operate::KEY_CODE::DPAD_RIGHT,       //RIGHT           = 5,
    DS4_Operate::KEY_CODE::DPAD_DOWN,        //DOWN            = 6,
    DS4_Operate::KEY_CODE::DPAD_LEFT,        //LEFT            = 7,
    DS4_Operate::KEY_CODE::BTN_TRIANGLE,     //TRIANGLE_LONG   = 8,
    DS4_Operate::KEY_CODE::BTN_CIRCLE,       //CIRCLE_LONG     = 9,
    DS4_Operate::KEY_CODE::BTN_CROSS,        //CROSS_LONG      = 0xA,
    DS4_Operate::KEY_CODE::BTN_SQUARE,       //SQUARE_LONG     = 0xB,
    DS4_Operate::KEY_CODE::STICK_R_X_ANALOG, //STAR            = 0xC,
    DS4_Operate::KEY_CODE::RESERVED,         //UNK             = 0xD,
    DS4_Operate::KEY_CODE::STICK_L_X_ANALOG, //STAR_DOUBLE     = 0xE,
    DS4_Operate::KEY_CODE::STICK_L_X_ANALOG, //STAR_CHANCE     = 0xF,
    DS4_Operate::KEY_CODE::RESERVED,         //UNK             = 0x10,
    DS4_Operate::KEY_CODE::RESERVED,         //UNK             = 0x11,
    DS4_Operate::KEY_CODE::RESERVED,         //UNK             = 0x12,
    DS4_Operate::KEY_CODE::RESERVED,         //UNK             = 0x13,
    DS4_Operate::KEY_CODE::RESERVED,         //UNK             = 0x14,
    DS4_Operate::KEY_CODE::RESERVED,         //UNK             = 0x15,
    DS4_Operate::KEY_CODE::STICK_L_X_ANALOG, //STAR_LINE_START = 0x16,
    DS4_Operate::KEY_CODE::STICK_L_X_ANALOG, //STAR_LINE_END   = 0x17,
    DS4_Operate::KEY_CODE::RESERVED,         //UNK             = 0x18,
    DS4_Operate::KEY_CODE::BTN_TRIANGLE,     //TRIANGLE_RUSH   = 0x19,
    DS4_Operate::KEY_CODE::BTN_CIRCLE,       //CIRCLE_RUSH     = 0x1A,
    DS4_Operate::KEY_CODE::BTN_CROSS,        //CROSS_RUSH      = 0x1B,
    DS4_Operate::KEY_CODE::BTN_SQUARE,       //SQUARE_RUSH     = 0x1C,
};

static inline uint32_t convert_time_to_ms(uint32_t dsc_time){
    dsc_time = dsc_time/10;
    if(dsc_time%10 >= 5){
        return dsc_time/10+1;
    }else{
        return dsc_time/10;
    }
}

int DSC_OperateEmitter::emit(DSC_Info* dsc, DS4_Controller* ctrl, int time_offset, bool is_salt){
    if(!dsc || !ctrl){
        return -1;
    }
    std::list<TimeStamp*>::iterator it_ts = dsc->event_seq.begin();
    std::list<Inst*>::iterator it_inst;
    Note* cur_note;
    while(it_ts != dsc->event_seq.end()){
        //TimeStamp* ts = *it_ts;
        (*it_ts)->time = convert_time_to_ms((*it_ts)->time);
        m_basetime = (*it_ts)->time + time_offset;
        
        for(it_inst = (*it_ts)->inst_list.begin();
            it_inst != (*it_ts)->inst_list.end();
            ++it_inst){
            if((*it_inst)->itype == Inst::InstType::NOTE){
                cur_note = (Note*)(*it_inst)->idata;
                if(!is_note_valid(cur_note)){
                    perror("DSC to Controller interrupted: Invalid dsc note.\n");
                    return -1;
                }
                cur_note->note_time_offset = convert_time_to_ms(cur_note->note_time_offset);
                switch(cur_note->note_keycode){
                    case NOTE_KEYCODE::TRIANGLE:
                    case NOTE_KEYCODE::CIRCLE:
                    case NOTE_KEYCODE::CROSS:
                    case NOTE_KEYCODE::SQUARE:
                        emit_singal_btn(ctrl, time_offset, cur_note);
                        break;
                    case NOTE_KEYCODE::UP:
                    case NOTE_KEYCODE::RIGHT:
                    case NOTE_KEYCODE::DOWN:
                    case NOTE_KEYCODE::LEFT:
                        emit_dual_btn(ctrl, time_offset, cur_note);
                        break;
                    case NOTE_KEYCODE::TRIANGLE_LONG:
                    case NOTE_KEYCODE::CIRCLE_LONG:
                    case NOTE_KEYCODE::CROSS_LONG:
                    case NOTE_KEYCODE::SQUARE_LONG:
                        emit_hold(ctrl, time_offset, cur_note);
                        break;
                    case NOTE_KEYCODE::STAR:
                        emit_star(ctrl, time_offset, cur_note);
                        break;
                    case NOTE_KEYCODE::STAR_DOUBLE:
                    case NOTE_KEYCODE::STAR_CHANCE:
                        emit_star_dual(ctrl, time_offset, cur_note);
                        break;
                    case NOTE_KEYCODE::STAR_LINE_START:
                    case NOTE_KEYCODE::STAR_LINE_END:
                        emit_star_line(ctrl, time_offset, cur_note);
                        break;
                    case NOTE_KEYCODE::TRIANGLE_RUSH:
                    case NOTE_KEYCODE::CIRCLE_RUSH:
                    case NOTE_KEYCODE::CROSS_RUSH:
                    case NOTE_KEYCODE::SQUARE_RUSH:
                        emit_rush(ctrl, time_offset, cur_note);
                        break;
                }
            }
        }
        ++it_ts;
    }
    return 0;
}

int DSC_OperateEmitter::is_note_valid(Note* note){
    if(((note->note_keycode >=0x10) && (note->note_keycode <=0x15)) ||
       (note->note_keycode > 0x1c))
        return 0;
    return 1;
}

int DSC_OperateEmitter::emit_singal_btn(DS4_Controller* ctrl, int time_offset, Note* note){
    DS4_Operate op;
    op.cb = nullptr;
    
    
    op.key = TBL_CONVERT[note->note_keycode];
    
    op.time_left_ms = m_basetime + note->note_time_offset + BTN_SINGLE_PUSH_ADVANCE_TIME_MS;
    
    if((op.key == m_last_emit_key) && ((op.time_left_ms - m_last_emit_time) < BTN_SINGLE_EXCHANGE_THRES)){
        op.key = BTN_CONVERT_R2L(op.key);
    }
    
    m_last_emit_key = op.key;
    m_last_emit_time = op.time_left_ms;
    
    op.val = 1;
    ctrl->insert_operate(op); // down
    
    
    
    op.val = 0;
    op.time_left_ms += BTN_SINGLE_HOLD_TIME_MS;
    ctrl->insert_operate(op); // up
    
    return 0;
}

int DSC_OperateEmitter::emit_dual_btn(DS4_Controller* ctrl, int time_offset, Note* note){
    DS4_Operate op;
    op.cb = nullptr;
    
    op.key = TBL_CONVERT[note->note_keycode];
    op.val = 1;
    op.time_left_ms = m_basetime + note->note_time_offset + BTN_DUAL_PUSH_ADVANCE_TIME_MS;
    ctrl->insert_operate(op); // down DPAD
    op.key = BTN_CONVERT_L2R(op.key);
    ctrl->insert_operate(op); // down BTN
    
    op.val = 0;
    op.time_left_ms += BTN_DUAL_HOLD_TIME_MS;
    ctrl->insert_operate(op); // up BTN
    op.key = BTN_CONVERT_R2L(op.key);
    ctrl->insert_operate(op); // up DPAD
    
    return 0;
}

int DSC_OperateEmitter::emit_hold(DS4_Controller* ctrl, int time_offset, Note* note){
    DS4_Operate op;
    op.cb = nullptr;
    
    op.key = TBL_CONVERT[note->note_keycode];
    if(note->note_hold_length != 0xffffffff) {
        op.val = 1;
        op.time_left_ms = m_basetime + note->note_time_offset + BTN_HOLD_ADVANCE_TIME_MS;
    }else{
        op.val = 0;
        op.time_left_ms = m_basetime + note->note_time_offset + BTN_HOLD_RELEASE_DELAY_TIME_MS;
    }

    ctrl->insert_operate(op); // up
    
    return 0;
}

int DSC_OperateEmitter::emit_star(DS4_Controller* ctrl, int time_offset, Note* note){
    assert(STAR_SLIDE_WINDOW_MS%STAR_SLIDE_STEP_N == 0);
    DS4_Operate op;
    op.key = TBL_CONVERT[note->note_keycode];
    op.cb = nullptr;
    
    int window = STAR_SLIDE_WINDOW_MS, step = STAR_SLIDE_STEP_N;
    int operate_interval = window/step;
    int operate_dac_step = (STICK_DAC_MAX-STICK_DAC_MID)/step;
    int dac = STICK_DAC_MID;
    int start_time = m_basetime+note->note_time_offset-(operate_interval/STAR_SLIDE_TRIGGE_POINT_DIV_N)+STAR_SLIDE_ADVANCE_TIME_MS;
    op.val = dac;
    op.time_left_ms = start_time;
    ctrl->insert_operate(op);
    for(int i = 0;i<step;++i){
        op.val += operate_dac_step;
        op.time_left_ms += operate_interval;
        ctrl->insert_operate(op);
    }
    for(int i = 0;i<step-1;++i){
        op.val -= operate_dac_step;
        op.time_left_ms += operate_interval;
        ctrl->insert_operate(op);
    }
    op.val = STICK_DAC_MID;
    op.time_left_ms += operate_interval;
    ctrl->insert_operate(op);
    
    return 0;
}


int DSC_OperateEmitter::emit_star_dual(DS4_Controller* ctrl, int time_offset, Note* note){
    assert(STAR_DUAL_SLIDE_WINDOW_MS%STAR_DUAL_SLIDE_STEP_N == 0);
    DS4_Operate op;
    op.key = TBL_CONVERT[note->note_keycode];
    op.cb = nullptr;
    
    int window = STAR_DUAL_SLIDE_WINDOW_MS, step = STAR_DUAL_SLIDE_STEP_N;
    int operate_interval = window/step;
    int operate_dac_step = (STICK_DAC_MAX - STICK_DAC_MID)/step;
    int dac = STICK_DAC_MID;
    int start_time = m_basetime+note->note_time_offset-(operate_interval/STAR_DUAL_SLIDE_TRIGGE_POINT_DIV_N)+STAR_DUAL_SLIDE_ADVANCE_TIME_MS;
    op.val = dac;
    op.time_left_ms = start_time;
    ctrl->insert_operate(op);
    op.key = STIWCH_STICK(op.key);
    ctrl->insert_operate(op);
    
    for(int i = 0;i<step;++i){
        op.val += operate_dac_step;
        op.time_left_ms += operate_interval;
        ctrl->insert_operate(op);
        op.key = STIWCH_STICK(op.key);
        ctrl->insert_operate(op);
    }
    for(int i = 0;i<step-1;++i){
        op.val -= operate_dac_step;
        op.time_left_ms += operate_interval;
        ctrl->insert_operate(op);
        op.key = STIWCH_STICK(op.key);
        ctrl->insert_operate(op);
    }
    op.val = STICK_DAC_MID;
    op.time_left_ms += operate_interval;
    ctrl->insert_operate(op);
    op.key = STIWCH_STICK(op.key);
    ctrl->insert_operate(op);
    return 0;
}

int DSC_OperateEmitter::emit_star_line(DS4_Controller* ctrl, int time_offset, Note* note){
    assert(0);
    return 0;
}

int DSC_OperateEmitter::emit_rush(DS4_Controller* ctrl, int time_offset, Note* note){
    DS4_Operate op1;
    DS4_Operate op2;
    op1.cb = nullptr;
    op2.cb = nullptr;
    op1.key = TBL_CONVERT[note->note_keycode];
    op2.key = BTN_CONVERT_R2L(op1.key);
    
    op1.val = 1;
    op1.time_left_ms = m_basetime + note->note_time_offset + RUSH_PUSH_ADVANCE_TIME_MS;
    
    op2.val = 1;
    op2.time_left_ms = m_basetime + note->note_time_offset + RUSH_PUSH_ADVANCE_TIME_MS+(RUSH_INVERVAL_MS/2);
    
    int step = convert_time_to_ms(note->note_hold_length)/RUSH_INVERVAL_MS;
    step = step*2;
    for(int i = 0; i < step;++i){
        ctrl->insert_operate(op1);
        op1.val = !op1.val;
        op1.time_left_ms += RUSH_INVERVAL_MS/2;
        if(RUSH_IS_DUAL){
            ctrl->insert_operate(op2);
            op2.val = !op2.val;
            op2.time_left_ms += RUSH_INVERVAL_MS/2;
        }
        
    }
    return 0;
}

int DSC_OperateEmitter::salt_generator(bool is_salt){
    return 0;
}

