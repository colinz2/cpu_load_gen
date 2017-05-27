#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <sys/time.h>

uint64_t getCurrVs()
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

int main()
{
    uint64_t a, b;
    double s = 0;
    while (1) {
        a = getCurrVs();
        s = cal();
        b = getCurrVs();
        printf("s = %lf, b-a = %ld\n", s, b - a);
    }    
    return 0;
}

