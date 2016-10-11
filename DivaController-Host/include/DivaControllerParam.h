// Copyright (C) 2016 S117 <admin@0x10c.pw>
// Bitman Lab.
// Licensed under the terms of the GNU GPL, version 2
// http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt


#ifndef __H_DIVA_CONTROLLER_PARAM
#define __H_DIVA_CONTROLLER_PARAM

typedef struct __diva_control_param {
    enum DIFFICULTY {
        EASY = 0,
        NORMAL,
        HARD,
        EXTREME
    } difficulty;
    enum GAME_VERSION {
        F = 0,
        F2,
        X
    } version;
    int pv_no;
    int time_offset;
    int is_add_salt;
} diva_control_param;


#endif /* DivaControllerParam_h */
