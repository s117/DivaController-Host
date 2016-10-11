// Copyright (C) 2016 S117 <admin@0x10c.pw>
// Bitman Lab.
// Licensed under the terms of the GNU GPL, version 2
// http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt


#include "DivaController.h"
#include "CtrlParam_PreDefined.h"

int main(int argc, char* argv[]) {
    GlobalTimer1ms::get_instance();
    DivaController ctrl = DivaController();
    return ctrl.work(diva_param_Ending_Medley__Ultimate_Exquisite_Rampage_extreme);
}


