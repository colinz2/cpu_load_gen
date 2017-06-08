#ifndef _CPU_USAGE_H_
#define _CPU_USAGE_H_

typedef struct stat_cputime_t
{
    uint64_t user;        /* normal processes executing in user mode */      
    uint64_t nice;        /* niced processes executing in user mode */
    uint64_t system;      /* processes executing in kernel mode */   
    uint64_t idle;         
    uint64_t iowait;      /* waiting for I/O to complete */   
    uint64_t irq;         /* servicing interrupts */ 
    uint64_t softirq;     /* servicing softirqs */   
    uint64_t steal;       /* involuntary wait */
    uint64_t guest;       /* running a normal guest */  
    uint64_t guest_nice;  /* running a niced guest */
    struct timeval tv;
} stat_cputime_t;

typedef struct cpu_usagetime_t
{  
    uint64_t total;
    uint64_t load;
    uint64_t idle;
    uint64_t io;
    uint64_t system;
    uint64_t user;
    uint64_t irq;
    uint64_t guest;
    struct timeval tv;
} cpu_usagetime_t;

struct Crb
{
    volatile uint32_t curr;
    uint32_t size;
    uint32_t msize;
    void**   data;
};
typedef struct Crb Crb_t;


Crb_t* crb_creat(uint32_t size, uint32_t msize);

int get_cpu_usagetime(cpu_usagetime_t *usage);

int crb_get_offset(Crb_t* rcb, void* data, uint32_t offset);
int crb_get_curr(Crb_t* rcb, void* data);
uint32_t crb_set(Crb_t* rcb, void* data );

float cal_cpu_usage(cpu_usagetime_t *b, cpu_usagetime_t *a);
void cal_cpu_usage_result(cpu_usagetime_t *b, cpu_usagetime_t *a, int delay) ;

#endif