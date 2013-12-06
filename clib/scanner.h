#ifndef SCANNER_H_
#define SCANNER_H_

#define ERR_EXIT_TIME_FAIL 1 
#define EXIT_NORMAL 0


void scanner_init();

void scanner_stop();

void scanner_set_pps(int pps);

void new_point_set(int length);
void set_point_by_index(int index, int x, int y, char blank);
void activate_point_set();

int scanner_main();

#endif //SCANNER_H_

