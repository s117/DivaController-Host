//
//  main.cpp
//  DivaController-Host
//
//  Created by s117 on 16/9/30.
//  Copyright © 2016年 s117. All rights reserved.
//
#include "DivaController.h"
#include "CtrlParam_PreDefined.h"

int main(int argc, char* argv[]) {
    GlobalTimer1ms::get_instance();
    DivaController ctrl = DivaController();
    return ctrl.work(diva_param_Ending_Medley__Ultimate_Exquisite_Rampage_extreme);
}


