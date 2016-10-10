//
//  DivaControllerParam.h
//  DivaController-Host
//
//  Created by s117 on 16/10/10.
//  Copyright © 2016年 s117. All rights reserved.
//

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
