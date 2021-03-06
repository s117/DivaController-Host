// Copyright (C) 2016 S117 <admin@0x10c.pw>
// Bitman Lab.
// Licensed under the terms of the GNU GPL, version 2
// http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt


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
    DSC_OperateEmitter();
    int64_t emit(DSC_Info* dsc, DS4_Controller* ctrl, int time_offset, bool is_salt);
private:
    int is_note_valid(Note* note);
    int64_t emit_singal_btn(DS4_Controller* ctrl, int time_offset, Note* note);
    int64_t emit_dual_btn(DS4_Controller* ctrl, int time_offset, Note* note);
    int64_t emit_hold(DS4_Controller* ctrl, int time_offset, Note* note);
    int64_t emit_star(DS4_Controller* ctrl, int time_offset, Note* note);
    int64_t emit_star_dual(DS4_Controller* ctrl, int time_offset, Note* note);
    int64_t emit_star_line(DS4_Controller* ctrl, int time_offset, Note* note);
    int64_t emit_rush(DS4_Controller* ctrl, int time_offset, Note* note);
    int gen_salt_ms();

    bool m_salt_param;
    uint32_t m_basetime;
    DS4_Operate::KEY_CODE m_last_emit_key;
    uint32_t m_last_emit_time;

    //DS4_Controller* m_ctrl;
};
#endif /* DSC_Reader_hpp */
