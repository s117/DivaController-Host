// Copyright (C) 2016 S117 <admin@0x10c.pw>
// Bitman Lab.
// Licensed under the terms of the GNU GPL, version 2
// http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt


#ifndef SERIALPORT_H
#define SERIALPORT_H

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h> /* File control definitions */
#include <errno.h>
#include <termios.h> /* POSIX terminal control definitions */

#define SERIAL_OK                   0x00000000
#define SERIAL_CLOSE                0x00000001
#define SERIAL_ERROR_OPEN_FAIL      0x00000002
#define SERIAL_ERROR_SET_FAIL       0x00000003
#define SERIAL_ERROR_ILLEGAL_STATE  0x00000004
//#define CNEW_RTSCTS
class SerialPort {
public:
    SerialPort();
    ~SerialPort();
    int openport(const char* com, speed_t baudrate, bool rawRead);
    int closeport();
    ssize_t writeport(const void* buf, size_t n);
    ssize_t readport(void *buf, size_t nbytes);
    size_t readfix(void* buf, size_t nbytes);
    void flush();
    int setblock(bool isblock);
    int isopen();
    int getfd();
    int set_timeout(int timeout);
private:
    int serialfd;

};

#endif // SERIALPORT_H
