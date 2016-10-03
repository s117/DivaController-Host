#include "SerialPort.h"

SerialPort::SerialPort() {
    serialfd = -1;
}

SerialPort::~SerialPort() {
    closeport();
}

//open port with given param, and use 8N1, no flow control, non-block read mode default, return error code

int SerialPort::openport(const char* com, speed_t baudrate, bool rawRead) {
    struct termios options;
    if (!(serialfd < 0))
        return SERIAL_OK;
    //if ((serialfd = open(com, O_RDWR | O_NOCTTY)) < 0)
    if ((serialfd = open(com, O_RDWR)) < 0)
        return SERIAL_ERROR_OPEN_FAIL;
    tcgetattr(serialfd, &options);
    cfsetispeed(&options, baudrate);
    cfsetospeed(&options, baudrate);

    //control:
    options.c_cflag |= (CLOCAL | CREAD); // Enable the receiver and set local mode...

    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8; // 8 Data bit
    options.c_cflag &= ~PARENB; // No parity bit
    options.c_cflag &= ~CSTOPB; // 1 Stop bit


#ifdef CNEW_RTSCTS
    options.c_cflag &= ~CNEW_RTSCTS; // Disable hardware flow control, for some platform only
#endif

    //local:
    if (rawRead) {//raw input mode
        options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    } else {//classic input mode (line mode)
        options.c_lflag |= (ICANON | ECHO | ECHOE);
    }

    //input data:
    //options.c_iflag |= (INPCK | ISTRIP);// Perform parity check, and strip parity bit
    options.c_iflag &= ~INPCK; // Not perform parity check for input data
    options.c_iflag &= ~(IXON | IXOFF | IXANY); // Disable software flow control

    //output data:
    options.c_oflag &= ~OPOST; // Output raw data
    options.c_cc[VTIME] = 150; // Set timeout 15 seconds
    options.c_cc[VMIN] = 0; // Update the options and do it NOW
    if (tcsetattr(serialfd, TCSANOW, &options) != 0)// Apply setting
        return SERIAL_ERROR_SET_FAIL;
    else
        return SERIAL_OK;
}

//close port, return error code

int SerialPort::closeport() {
    if (!(serialfd < 0)) {
        close(serialfd);
        serialfd = -1;
    }
    return SERIAL_OK;

}

//write data to port, return bytes written

ssize_t SerialPort::writeport(const void* buf, size_t n) {
    if (serialfd < 0)
        return 0;
    return write(serialfd, buf, n);
}

//read from port, block mode can be setted, return bytes readed

ssize_t SerialPort::readport(void *buf, size_t nbytes) {
    if (serialfd < 0) {
        //printf("read error\n");
        return 0;
    } else {
        //printf("read ok,nbytes=%d\n",nbytes);
        return read(serialfd, buf, nbytes);
    }

}

size_t SerialPort::readfix(void* buf, size_t nbytes) {
    size_t cnt = 0;
    size_t readed;
    while (cnt < nbytes) {
        readed = read(serialfd, ((char*) buf) + cnt, 1);
        if (readed <= 0) {
            if (errno != EAGAIN)
                return cnt;
        } else
            cnt += readed;
    }
    if (cnt == nbytes)
        return nbytes;
    else
        return 0;
}

//flush data in buffer

void SerialPort::flush() {
    tcflush(this->serialfd, TCIFLUSH);
}

//set block mode, TRUE FOR BLOCK, FALSE FOR UNBLOCK, return error code

int SerialPort::setblock(bool isblock) {
    if (serialfd < 0)
        return SERIAL_ERROR_ILLEGAL_STATE;
    if (isblock) {
        fcntl(serialfd, F_SETFL, 0);
    } else {
        fcntl(serialfd, F_SETFL, FNDELAY);
    }
    return SERIAL_OK;
}

int SerialPort::isopen() {
    return (serialfd < 0) ? SERIAL_CLOSE : SERIAL_OK;
}

int SerialPort::getfd() {
    return serialfd;
}
// second = timeout / 10

int SerialPort::set_timeout(int timeout) {
    struct termios options;
    if (serialfd < 0)
        return SERIAL_ERROR_ILLEGAL_STATE;

    tcgetattr(serialfd, &options);

    options.c_cc[VTIME] = timeout; // Set timeout 15 seconds

    if (tcsetattr(serialfd, TCSANOW, &options) != 0)// Apply setting
        return SERIAL_ERROR_SET_FAIL;
    else
        return SERIAL_OK;
}
