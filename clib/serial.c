#include "serial.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>

#define PORT "/dev/ttyUSB0"
#define BAUD B1000000 //1 MBaud

// Serial port file descriptor
int fd_serial;

int init_serial(char* port) {
    struct termios port_settings;

    // Set serial port settings
    bzero(&port_settings, sizeof(port_settings));
    port_settings.c_cflag = CS8 | CLOCAL | CREAD; // 8n1
    port_settings.c_iflag = 0; // raw input mode 
    port_settings.c_oflag = 0; // raw output mode
    port_settings.c_lflag = 0;
    port_settings.c_cc[VMIN]=1;
    port_settings.c_cc[VTIME]=5;

    fd_serial = open(port, O_RDWR | O_NOCTTY);
    if (fd_serial < 0) return ERR_COULD_NOT_OPEN_PORT; 

    cfsetospeed(&port_settings, BAUD);
    cfsetispeed(&port_settings, BAUD);
    tcflush(fd_serial, TCIFLUSH);
    tcsetattr(fd_serial, TCSANOW, &port_settings);

    return SUCCESS; //success; port open!
}


unsigned char buf[4];

int serial_new_point(int x, int y, char blank) {
    int ret;

    if (x > POINT_MAX || y > POINT_MAX || x < 0 || y < 0) return ERR_POINT_OUT_OF_RANGE;

    buf[0] = ((((unsigned char) blank) & 0x01) << 4) | ((((unsigned int) x) >> 8) & 0x0F);
    buf[1] = (((unsigned int) x) & 0xFF);

    buf[2] = ((((unsigned int) y) >> 8) & 0x0F);
    buf[3] = (((unsigned int) y) & 0xFF);

//printf("debug: port write: 0x%02X, 0x%02X, 0x%02X, 0x%02X\n", buf[0], buf[1], buf[2], buf[3]);

    ret = write(fd_serial, buf, sizeof(buf));
    if (ret < 0) return ERR_BAD_WRITE;
    return SUCCESS;
}

