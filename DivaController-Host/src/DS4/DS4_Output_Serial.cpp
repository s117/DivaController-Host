// Copyright (C) 2016 S117 <admin@0x10c.pw>
// Bitman Lab.
// Licensed under the terms of the GNU GPL, version 2
// http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt


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


