//
//  DSC_Reader.hpp
//  DivaController-Host
//
//  Created by s117 on 16/9/30.
//  Copyright © 2016年 s117. All rights reserved.
//

#ifndef __H_OPERATE_EMITTER
#define __H_OPERATE_EMITTER

#include <stdio.h>
#include "DSC_Info.h"
#include <stdio.h>
#include "PVSC_Parser.h"
#include "DS4_Controller.h"
#include "list.h"

typedef struct __section_data {
    struct list_head list;
    uint32_t section_offset;
} section_data_t;


class DSC_OperateEmitter {
public:
    int emit(DSC_Info* dsc, DS4_Controller* ctrl, int time_offset, bool is_salt);
private:
    int is_note_valid(Note* note);
    int emit_singal_btn(DS4_Controller* ctrl, int time_offset, Note* note);
    int emit_dual_btn(DS4_Controller* ctrl, int time_offset, Note* note);
    int emit_hold(DS4_Controller* ctrl, int time_offset, Note* note);
    int emit_star(DS4_Controller* ctrl, int time_offset, Note* note);
    int emit_star_dual(DS4_Controller* ctrl, int time_offset, Note* note);
    int emit_star_line(DS4_Controller* ctrl, int time_offset, Note* note);
    int emit_rush(DS4_Controller* ctrl, int time_offset, Note* note);
    int salt_generator(bool is_salt);

    int m_salt_param;
    uint32_t m_basetime;
    DS4_Operate::KEY_CODE m_last_emit_key;
    uint32_t m_last_emit_time;

    //DS4_Controller* m_ctrl;
};
#endif /* DSC_Reader_hpp */
