#include "scanner.h"

#include "serial.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#define BILLION  1000000000L

typedef struct {
    int x;
    int y;
    char blank;
} laserpoint;

pthread_mutex_t lock;


laserpoint* active; // Active laser point scan array
int active_size;
laserpoint* staging; // Next-up scan array
int staging_size;

char stop_flag;
int setting_pps;

void scanner_init() {
    stop_flag = 0;
    setting_pps = 1;

    active = NULL;
    active_size = 0;
    staging = NULL;
    staging_size = 0;

    (void) pthread_mutex_init(&lock, NULL);

    // Initialize laser scanning thread with highest priority
    pthread_t thread1;
    pthread_create(&thread1, NULL, (void *) &scanner_main, NULL);

    pthread_attr_t thAttr;
    int policy = 0;
    int max_prio_for_policy = 0;

    pthread_attr_init(&thAttr);
    pthread_attr_getschedpolicy(&thAttr, &policy);
    max_prio_for_policy = sched_get_priority_max(policy);

    pthread_setschedprio(thread1, max_prio_for_policy);
    pthread_attr_destroy(&thAttr);
}

void scanner_stop() {
    pthread_mutex_lock(&lock);
    stop_flag = 1;
    pthread_mutex_unlock(&lock);
}

void scanner_set_pps(int pps) {
    pthread_mutex_lock(&lock);
    setting_pps = pps;
    pthread_mutex_unlock(&lock);
}

void new_point_set(int length) {
    if (staging != NULL) {
        //printf("FREE PT\n");
        free(staging);
    }
    staging = (laserpoint*) malloc(sizeof(laserpoint) * length);
    staging_size = length;
    //printf("Init point set of size %d\n", length);
}

void set_point_by_index(int index, int x, int y, char blank) {
    staging[index].x = x;
    staging[index].y = y;
    staging[index].blank = blank;
}

void activate_point_set() {
    pthread_mutex_lock(&lock);
    active = staging;
    active_size = staging_size;
    staging = NULL;
    staging_size = 0;
    pthread_mutex_unlock(&lock);
    //printf("activated point set\n");
}

// Helper: add nanoseconds to a time struct
void add_nanoseconds(struct timespec* t, long nanoseconds) {
    long newnano = t->tv_nsec + nanoseconds;
    t->tv_nsec = newnano % BILLION;
    t->tv_sec += newnano / BILLION;
}

int timeval_subtract(struct timespec *result, struct timespec *start, struct timespec *end) {
    if (end->tv_nsec - start->tv_nsec < 0) {
        result->tv_sec = end->tv_sec - start->tv_sec - 1;
        result->tv_nsec = BILLION + end->tv_nsec - start->tv_nsec;
    } else {
        result->tv_sec = end->tv_sec - start->tv_sec;
        result->tv_nsec = end->tv_nsec - start->tv_nsec;
    }
    return result->tv_sec >= 0;
}

int missed_deadlines = 0;

// Helper: sleep until a system time
int sleep_until(struct timespec* t) {
    struct timespec now;
    struct timespec sleeptime; //dummy;
    int spin = 0;

    while(1) {
        if (clock_gettime(CLOCK_REALTIME, &now) < 0) return ERR_EXIT_TIME_FAIL;

        if (!timeval_subtract(&sleeptime, &now, t)) {
            // Done waiting
            if (spin == 0) {
                missed_deadlines++;
            }
            break;
        }
        spin++;
    }

    return EXIT_NORMAL;
}

#define SQDIST(x,y) (x*x+y*y)

int scanner_main() {
    struct timespec cur;

    if (clock_gettime(CLOCK_REALTIME, &cur) < 0) return ERR_EXIT_TIME_FAIL;

    int scan_index = 0;
    int size = 0;
    int blank_override = 0;
    laserpoint* scan_array = NULL;

    while(!stop_flag) {

        pthread_mutex_lock(&lock);
        if (scan_array != active) { // Switching frames!
            int px, py, min, min_index, i;
            min_index = 0;
            if (scan_array != NULL) {
                px = scan_array[scan_index].x; // present coordinate
                py = scan_array[scan_index].y;

                // Find the closest point in the new array to the currently scanned point
                if (active != NULL) {
                    min = SQDIST(px - active[0].x, py - active[0].y);
                    for (i = 1; i < active_size; i++) {
                        int m = SQDIST(px - active[i].x, py - active[i].y);
                        if (m < min) {
                            min = m;
                            min_index = i;
                        }
                    }
                }
            }

            // Swap out the frame buffer and free old buffer
            if (scan_array != NULL) free(scan_array);
            scan_array = active;
            scan_index = min_index; // start scanning at the closest point in the new frame
            size = active_size;

            // Do a blank scan to the first point of the new frame
            blank_override = 1;
        }
        pthread_mutex_unlock(&lock);
    
        if (scan_array != NULL) {
            //printf("Scan of index %d\n", scan_index);
            serial_new_point(scan_array[scan_index].x, scan_array[scan_index].y, blank_override ? 1 /*force blanking*/ : scan_array[scan_index].blank);
            scan_index++;

            // wrap scan if overflow
            if (scan_index > size) {
                scan_index = 0;
            }
        }
        blank_override = 0;

        // sleep until next iteration
        add_nanoseconds(&cur, BILLION/setting_pps);
        sleep_until(&cur);
    }

    pthread_mutex_lock(&lock);
    if (active != NULL) {
        free(active);
        active = NULL;
    }
    pthread_mutex_unlock(&lock);

    return EXIT_NORMAL;
}
