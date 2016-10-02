//
//  DSC_Reader.cpp
//  DivaController-Host
//
//  Created by s117 on 16/9/30.
//  Copyright © 2016年 s117. All rights reserved.
//

#ifndef __H_DS4_OUTPUT_SERIAL
#define __H_DS4_OUTPUT_SERIAL
#include "DS4_Output.h"
#include "SerialPort.h"

#define DS4_OUTPUT_SERIAL_COM_N "/dev/cu.usbserial"
#define DS4_OUTPUT_SERIAL_BUAD  B230400

class DS4_Output_Serial : public DS4_Output{
public:
    DS4_Output_Serial();
    ~DS4_Output_Serial();
    virtual int write(const char* data, size_t len);
    virtual void flush();
private:
    SerialPort *serial;
};

#endif
