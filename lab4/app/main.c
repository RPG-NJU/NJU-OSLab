#include "lib.h"
#include "types.h"

#define MODE 2

int data = 0;

int uEntry(void) 
{

#if MODE == 1
    int i = 4;
    int ret = 0;
    sem_t sem;

    // printf("Pid %d\n", getpid());
    // sleep(128);

    printf("Father Process: Semaphore Initializing.\n");
    ret = sem_init(&sem, 2);
    if (ret == -1) // 如果进入这个分支，则说明fork失败，理论上是不可能的
    {
        printf("Father Process: Semaphore Initializing Failed.\n");
        exit();
    }

    ret = fork();
    if (ret == 0) 
    {
        while(i != 0) 
        {
            i --;
            // printf("Pid %d\n", getpid());
            printf("Child Process: Semaphore Waiting.\n");
            sem_wait(&sem);
            printf("Child Process: In Critical Area.\n");
        }
        printf("Child Process: Semaphore Destroying.\n");
        sem_destroy(&sem);
        exit();
    }
    else if (ret != -1) 
    {
        while( i != 0) 
        {
            i --;
            // printf("Pid %d\n", getpid());
            printf("Father Process: Sleeping.\n");
            sleep(128);
            printf("Father Process: Semaphore Posting.\n");
            sem_post(&sem);
        }
        printf("Father Process: Semaphore Destroying.\n");
        sem_destroy(&sem);
        exit();
    }
#endif

#if MODE == 2
    printf("[Start Test Producer & Consumer Program]\n");

    sem_t buffer, lock;
    int fork_time = 6;
    int ret = 0;
    int k = 1;
    int j = 0;
    int i = 0;
    
    sem_init(&buffer, 0); sem_init(&lock, 1);

    for (int i = 0; i < fork_time; ++i)
    {
        ret = fork();
        if (ret == -1)
            exit();
    }
    
    if (getpid() == 2 || getpid() == 3)
    {
        j = getpid() - 1; // 编号从1开始进行编号
        i = getpid();

        for ( ; k <= 8; )
        {
            sleep(64);
            printf("pid %d, producer %d, try lock, product %d\n", i, j, k);
            sem_wait(&lock);
            printf("pid %d, producer %d, locked\n", i, j);
            //sleep(16);
            sem_post(&buffer);
            sleep(12);
            printf("pid %d, prodecer %d, produce %d\n", i, j, k);
            sem_post(&lock);
            printf("pid %d, producer %d, unlock\n", i, j);
            ++k;
        }
        exit();
    }

    else if (getpid() > 3 && getpid() < 8)
    {
        j = getpid() - 3; // 编号从1开始进行编号
        i = getpid();

        for ( ; k <= 4; )
        {
            printf("pid %d, consumer %d, try consume, product %d\n", i, j, k);
            sem_wait(&buffer);
            printf("pid %d, consumer %d, try lock, product %d\n", i, j, k);
            sem_wait(&lock);
            printf("pid %d, consumer %d, locked\n", i, j);
            sleep(16);
            printf("pid %d, consumer %d, consumed, product %d\n", i, j, k);
            sem_post(&lock);
            printf("pid %d, consumer %d, unlock\n", i, j);
            sleep(160);
            ++k;
        }
        exit();
    }

    // if (getpid() == 0)
    //     printf("[End Test Producer & Consumer Program]\n");
    exit();
#endif
    return 0;
}
