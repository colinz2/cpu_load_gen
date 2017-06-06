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
#include <sys/types.h>
#include <sys/syscall.h>  

#include "cpu_usage.h"

#define gettid_syscall() syscall(__NR_gettid)  

struct ThreadInfo
{
    pthread_t tid;
    int core_num;
    int run;
};

static pthread_mutex_t g_lock;
static pthread_cond_t  g_cond;

static volatile int g_running = 1;
static volatile int g_waiting = 1;
static float g_cpu_usage = 66.0;

static void 
pthreadCall(const char* label, int result) {
    if (result != 0) {
        fprintf(stderr, "pthread %s: %s\n", label, strerror(result));
        abort();
    }
}

static double cal(void)
{
    int i = 0;
    double y = 0;
    double x = 33333333;
    for (i = 0; i < 1000; i++) {
        y += sqrt(x) + sqrt(x -i) + sqrt(x + i) + sin(y);
        x++;
    }
    return y;
}

static void* load(void* arg)
{
    cpu_set_t mask;
    int core = (int)(*(int*)arg);

    printf("thread (pid:%d)(tid:%ld)(thread id:%ld) will run at core id=%d \n", getpid(), syscall(__NR_gettid), pthread_self(), core);

    CPU_ZERO(&mask);
    CPU_SET(core, &mask);
    if (pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask) < 0) {
        fprintf(stderr, "set thread affinity failed\n");
        pthread_exit(NULL);
    }

    while (g_running) {
        pthreadCall("g_lock lock", pthread_mutex_lock(&g_lock));
        while (g_waiting) {
            pthreadCall("g_cond wait", pthread_cond_wait(&g_cond, &g_lock));
        }
        pthreadCall("g_lock unlock", pthread_mutex_unlock(&g_lock));
        cal();
    }
    pthread_exit(NULL);
    return NULL;
}

static float cal_cpu(Crb_t* crb)
{
    cpu_usagetime_t a, b;
    float usage_rate = g_cpu_usage;
    get_cpu_usagetime(&a);
    crb_set(crb, &a);
    if (crb_get_offset(crb, &b, 25) == 0) {
        usage_rate = cal_cpu_usage(&b, &a);
    }
    return usage_rate;
}

int main(int argc, char const *argv[])
{
    int i, flag;
    int num = sysconf(_SC_NPROCESSORS_CONF);
    struct ThreadInfo* thread_info = NULL;
    float rate;

    if (argc == 2) {
        g_cpu_usage = (float)atoi(argv[1]);
    }
    printf("g_cpu_usage= %f\n", g_cpu_usage);

    thread_info = calloc(sizeof(struct ThreadInfo), num);
    if (thread_info == NULL) {
        fprintf(stderr, "thread_info is NULL\n");
        exit(-1);
    }

    pthreadCall("g_lock init", pthread_mutex_init(&g_lock, NULL));
    pthreadCall("g_cond init", pthread_cond_init(&g_cond, NULL));

    for (i = 0; i < num; i++) {
        thread_info[i].core_num = i;
        thread_info[i].run = 0;
        if (pthread_create(&thread_info[i].tid, NULL, (void *)load, &thread_info[i].core_num) != 0) {
            fprintf(stderr, "thread create failed\n");
            break;
        }
        thread_info[i].run = 1;
    }

    Crb_t* crb = crb_creat(256, sizeof(cpu_usagetime_t));
    if (NULL == crb) {
        fprintf(stderr, "fail to create crb\n");
        exit(-1);
    }

    while (1) {
        rate = cal_cpu(crb);
        if (rate > g_cpu_usage) {
            if (rate - g_cpu_usage > 0.9) {
                flag = 1;
            } 
        } else if (rate < g_cpu_usage) {
            if (g_cpu_usage - rate > 0.9) {
                flag = 0;
            } 
        }

        pthreadCall("g_lock lock", pthread_mutex_lock(&g_lock));
        g_waiting = flag;
        pthreadCall("g_lock unlock", pthread_mutex_unlock(&g_lock));
        pthreadCall("g_cond broadcast", pthread_cond_broadcast(&g_cond));
        usleep(1000 * 20);
        //printf("g_waiting = %d , flag = %d, %f, %f\n", g_waiting, flag, rate - g_cpu_usage, fabsf(rate - g_cpu_usage));
    }

    for (i = 0; i < num; i++) {
        if (thread_info[i].run) {
            pthread_join(thread_info[i].tid, NULL);
        }
    }

    pthreadCall("g_lock destryoy", pthread_mutex_destroy(&g_lock));
    pthreadCall("g_cond destryoy", pthread_cond_destroy(&g_cond));
    free(thread_info);
    return 0;
}

