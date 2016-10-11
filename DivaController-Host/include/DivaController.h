// Copyright (C) 2016 S117 <admin@0x10c.pw>
// Bitman Lab.
// Licensed under the terms of the GNU GPL, version 2
// http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt


#ifndef __H_DIVA_CONTROLLER
#define __H_DIVA_CONTROLLER

#include "DivaControllerParam.h"
#include "DS4_Controller.h"
#include <stdint.h>

class DivaController {
public:
    int work(diva_control_param& param);
private:
    const char* generate_dsc_path(diva_control_param& param);
    void reset_ds4_btn(DS4_Controller* controller);
    int64_t get_first_note_time(DSC_Info *info);
    char buffer_dsc_path[PATH_MAX+1];
};
#endif
