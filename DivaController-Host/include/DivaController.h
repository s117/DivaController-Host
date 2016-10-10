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
