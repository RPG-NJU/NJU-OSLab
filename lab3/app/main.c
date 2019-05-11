#include "lib.h"
#include "types.h"
#include "pthread.h"
// #include "stdio.h"
// #include "assert.h"

// #define GRPUMAINDEBUG

int data = 0;
int main(void);

int uEntry(void) // 用户测试部分的函数
{
    // while(1);
    // printf("<uMain START>\n");
    // printf("?\n");
    // assert(0);
    // int x = 100000000;
    // asm volatile("movl %0, %%eip"::"m"(x));
    // asm volatile("int $0x20");
    // while(1);
#ifdef GRPUMAINDEBUG
    printf("<uMain START>\n");
    // while(1) printf("TEST\n");
#endif
    int ret = fork();
#ifdef GRPUMAINDEBUG
    // while(1);
    printf("fork ret == %d\n", ret);
    // while(1);
#endif
    // while(1);
    int i = 8;
    if (ret == 0)
    {
#ifdef GRPUMAINDEBUG
        printf("here is [if case] == 0\n");
        //while(1);
#endif
        data = 2;
        // while(1);
        while(i != 0)
        {
            i--;
            printf("child Process: %d, %d;\n", data, i);
            
            sleep(128);
            
        }
        // for (int j = 0; j < 100; ++j)
        // {
        //     printf("{2}");
        // }
        // while(1);
        exit();
        // printf("can't be here\n");
    }
    else if(ret != -1)
    {
#ifdef GRPUMAINDEBUG
        printf("here is ret != 0\n");
        // printf("what happen\n");
        // int what = 0;
        // while(1);
#endif
        pthread_initial();
        main();
    }
    return 0;
}
