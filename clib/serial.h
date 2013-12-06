#ifndef SERIAL_H_
#define SERIAL_H_

#define SUCCESS 0
#define ERR_COULD_NOT_OPEN_PORT 1
#define ERR_BAD_WRITE   2
#define ERR_POINT_OUT_OF_RANGE  3

#define POINT_MAX   0x0FFF

int init_serial(char* port);

int serial_new_point(int x, int y, char blank);

#endif //SERIAL_H_



