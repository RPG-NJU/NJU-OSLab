#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h> // 需要这个库的支持，因为Linux本来是不支持多线程的

// #define GRPDEBUG
// #define PRINT

#define N (unsigned long long)10000000
#define THREAD_NUM 2
#define PRE_THREAD_NUM (N/THREAD_NUM)

double GetTime();

void *compute_pi(void *ptr);

unsigned long long data[THREAD_NUM]; // 用来记录所有的数据，在开始之初，用于记录种子

int main()
{
    double time_begin, time_end; // 这是时间的记录
    unsigned long long total_data = 0; // 作为最后的数据
    struct timeval time;

    time_begin = GetTime();

#ifdef GRPDEBUG
    printf("Now Time : %.10f\n", time_begin);
#endif

    pthread_t my_thread[THREAD_NUM]; // 作为线程的存储结构
    pthread_attr_t attr; // 线程属性


    pthread_attr_init(&attr);
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM); // 绑定，有助于加快线程的速度

    for (int i = 0; i < THREAD_NUM; ++i)
    {
        gettimeofday(&time, NULL);
        data[i] = time.tv_usec;
#ifdef GRPDEBUG
        printf("seed %d : %llu\n", i, data[i]);
#endif

        pthread_create(&my_thread[i], &attr, compute_pi, (void*)&data[i]);
    }


    for (int i = 0; i < THREAD_NUM; ++i)
    {
        pthread_join(my_thread[i], NULL);
        total_data += data[i];
#ifdef PRINT
        printf("Thread %d data : %llu\n", i, data[i]);
#endif
    }

    double pi = ((double)total_data / (PRE_THREAD_NUM * THREAD_NUM)) * 4.0;
    time_end = GetTime();

    printf("Compute %llu Times\n", (PRE_THREAD_NUM * THREAD_NUM));
    printf("Time : %.10f sec\n", time_end - time_begin);
    printf("Pi : %.10f\n", pi);
    
    return 0;
}


double GetTime()
{
    struct timeval time;
    gettimeofday(&time, NULL);
    return (double)time.tv_sec + (double)time.tv_usec / 1000000.0;
}

void *compute_pi(void *ptr)
{
    double time_begin, time_end;
    time_begin = GetTime();
    unsigned long long seed_long = 0; // 种子    
    unsigned int seed = 0;
    unsigned long long *count = (unsigned long long *)ptr; // 将参数进行类型转换
    seed_long = *count; // 此时这个位置上放置的是种子
    seed = (unsigned int)seed_long;
#ifdef GRPDEBUG
        printf("thread's seed : %u\n", seed);
#endif
    double x = 0, y = 0;
    *count = 0;
    for (unsigned long long i = 0; i < PRE_THREAD_NUM; ++i)
    {
        x = (double)(rand_r(&seed)) / (double)(RAND_MAX);
        y = (double)(rand_r(&seed)) / (double)(RAND_MAX);
        if (x * x + y * y <= 1)
            ++(*count);
        
    }
    time_end = GetTime();

#ifdef PRINT
    printf("This thread's Time : %.10f sec\n", time_end - time_begin);
#endif
    pthread_exit(0);
    //return;
}