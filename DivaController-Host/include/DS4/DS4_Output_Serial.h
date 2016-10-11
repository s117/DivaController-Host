// Copyright (C) 2016 S117 <admin@0x10c.pw>
// Bitman Lab.
// Licensed under the terms of the GNU GPL, version 2
// http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt


#ifndef __H_DS4_OUTPUT_SERIAL
#define __H_DS4_OUTPUT_SERIAL
#include "DS4_Output.h"
#include "SerialPort.h"

#define DS4_OUTPUT_SERIAL_COM_N "/dev/cu.usbserial"
#define DS4_OUTPUT_SERIAL_BUAD  B115200

class DS4_Output_Serial : public DS4_Output {
public:
    DS4_Output_Serial();
    ~DS4_Output_Serial();
    virtual int write(const char* data, size_t len);
    virtual void flush();
private:
    SerialPort *serial;
};

#endif
