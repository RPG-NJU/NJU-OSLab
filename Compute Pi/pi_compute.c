#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h> // 需要这个库的支持，因为Linux本来是不支持多线程的
#include <fcntl.h>

// #define GRPDEBUG

#define N (unsigned long long)10000000000
#define THREAD_NUM 1
#define PRE_THREAD_NUM (N/THREAD_NUM)

double GetTime()
{
    struct timeval time;
    gettimeofday(&time, NULL);
    return (double)time.tv_sec + (double)time.tv_usec / 1000000.0;
}


int main()
{
    struct timeval time;
    double time_begin, time_end;
    gettimeofday(&time, NULL);
    unsigned int seed = time.tv_usec;
    unsigned long long count = 0;
    int randNum = 0;
    time_begin = GetTime();
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd == -1)
        exit(-1);

    // seed = 1;
    // seed_long = *count; // 此时这个位置上放置的是种子
    // seed = (unsigned int)seed_long;
    double x = 0, y = 0;
    unsigned int seed1, seed2, seed3, seed4;
    for (unsigned long long i = 0; i < N; ++i)
    {
        (unsigned int)read(fd, (char *)&seed1, sizeof(int));
        (unsigned int)read(fd, (char *)&seed2, sizeof(int));

        //printf("seed1 = %u, seed2 = %u\n", seed1, seed2);
        //seed3 = (unsigned int)read(fd, (char *)&randNum, sizeof(int));
        //seed4 = (unsigned int)read(fd, (char *)&randNum, sizeof(int));
        //x = (double)(rand_r(&seed) % RAND_MAX) / (double)(RAND_MAX);
        //y = (double)(rand_r(&seed) % RAND_MAX) / (double)(RAND_MAX);
        x = (double)seed1 / __UINT32_MAX__;
        y = (double)seed2 / __UINT32_MAX__;
        if (x * x + y * y <= 1)
            ++count;
        
    }

    close(fd);

    time_end = GetTime();
    printf("%llu\n", count);
    double pi = ((double)count / N) * 4.0;
    
    printf("Compute %llu Times\n", N);
    printf("Time : %.10f sec\n", time_end - time_begin);
    printf("Pi : %.10f\n", pi);

    return 0;
    //pthread_exit(0);
    //return;
}