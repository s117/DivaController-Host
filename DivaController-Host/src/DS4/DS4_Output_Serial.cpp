//
//  DSC_Reader.cpp
//  DivaController-Host
//
//  Created by s117 on 16/9/30.
//  Copyright © 2016年 s117. All rights reserved.
//

#include "DS4_Output_Serial.h"
DS4_Output_Serial::DS4_Output_Serial() {
    serial = new SerialPort();
    serial->openport(DS4_OUTPUT_SERIAL_COM_N, DS4_OUTPUT_SERIAL_BUAD, true);
}

DS4_Output_Serial::~DS4_Output_Serial() {
    serial->flush();
    serial->closeport();
}

int DS4_Output_Serial::write(const char* data, size_t len) {
    serial->writeport(data, len);
    return 0;
}
void DS4_Output_Serial::flush() {
    serial->flush();
}


