#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define __USE_GNU  
#include <sched.h>
#include <pthread.h>
#include <sys/time.h>

uint64_t getCurrTV()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec *1000 + tv.tv_usec /1000;
}

double cal()
{
    int i = 0;
    double y = 0;
    double x = 33333333;
    for (i = 0; i < 10000; i++) {
        y += sqrt(x) + sqrt(x -i) + sqrt(x + i) + sin(y);
        x++;
    }
    return y;
}

void *myfun(void *arg)
{
    cpu_set_t mask;
    cpu_set_t get;
    char buf[256];
    int i;
    int j;
    int num = sysconf(_SC_NPROCESSORS_CONF);
    printf("system has %d processor(s)\n", num);

    for (i = 0; i < num; i++) {
        CPU_ZERO(&mask);
        CPU_SET(i, &mask);
        if (pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask) < 0) {
            fprintf(stderr, "set thread affinity failed\n");
        }
        CPU_ZERO(&get);
        if (pthread_getaffinity_np(pthread_self(), sizeof(get), &get) < 0) {
            fprintf(stderr, "get thread affinity failed\n");
        }
        for (j = 0; j < num; j++) {
            if (CPU_ISSET(j, &get)) {
                printf("thread %d is running in processor %d\n", (int)pthread_self(), j);
            }
        }
        j = 0;
        while (j++ < 100000000) {
            memset(buf, 0, sizeof(buf));
        }
    }
    pthread_exit(NULL);
}

int main()
{
    uint64_t a, b;
    double s = 0;
    pthread_t tid;
    if (pthread_create(&tid, NULL, (void *)myfun, NULL) != 0) {
        fprintf(stderr, "thread create failed\n");
        return -1;
    }
    
    while (1) {
        a = getCurrTV();
        s = cal();
        b = getCurrTV();
        printf("s = %lf, b-a = %ld\n", s, b - a);
    }  
    pthread_join(tid, NULL);
    return 0;
}

