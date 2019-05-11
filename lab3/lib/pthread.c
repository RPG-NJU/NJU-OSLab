#include "pthread.h"
#include "lib.h"
/*
 * pthread lib here
 * 用户态多线程写在这
 */

// 谢天谢地不是在内核态
// 这是一个完全在用户态完成的线程
 
ThreadTable tcb[MAX_TCB_NUM];
#define CHANGE_EXCHANGE 1
#define CHANGE_GETONE 2
/*
struct ThreadTable 
{
    uint32_t stack[MAX_STACK_SIZE];
    struct Context cont; // 类似进程控制块中的所有寄存器regs
    uint32_t retPoint;              // the entry to exit the thread
    uint32_t pthArg;                // the arg to pass into thread
    uint32_t stackTop;
    int state;
    uint32_t pthid;
    uint32_t joinid;
};
*/
int current; // 当前线程

void change_pthread(int mode);

void pthread_initial(void)
{    
    int i;
    for (i = 0; i < MAX_TCB_NUM; i++) 
    {
        tcb[i].state = STATE_DEAD;
        tcb[i].joinid = -1;
    }
    tcb[0].state = STATE_RUNNING;
    tcb[0].pthid = 0;
    current = 0; // main thread
    return;
}

/*
和进程一样，线程的上下文包括寄存器和堆栈
本次实验的进程切换通过TSS段和硬件支持完成，而用户态的线程切换不能通过中断，也没有硬件配合
我们需要通过直接操作%esp寄存器和某些能修改%eip的指令（如call、jmp、ret）或编译后能生成这些指令的语句（return）完成
如果你还有其他思路，请在实验报告时阐述你的思路和实现
*/
int pthread_create(pthread_t *thread, const pthread_attr_t *attr, 
                   void *(*start_routine)(void *), void *arg)
{
    // 在测试样例中，attr参数都是NULL，这个属性参数在我们的实验中就不进行模拟了，作为空参数维护了与标准库的统一性而已
    // pthread_t类型是一个unsigned long int
    // start_routine是函数开始的地址（开始例程）
    int i = 0; // 用于选择线程控制块的索引值
    for (; i < MAX_TCB_NUM; ++i)
    {
        if (tcb[i].state == STATE_DEAD)
            break;
    }
    if (i == MAX_TCB_NUM)
        return -1; // 线程创建失败的例子
    
    *thread = i; // 进程的标识符
    
    tcb[i].pthid = i;
    tcb[i].stackTop = (uint32_t)&(tcb[i].stack);
    tcb[i].pthArg = (uint32_t)arg;
    // attr is nothing
    
    tcb[i].cont.edi = 0;
    tcb[i].cont.esi = 0;
    tcb[i].cont.ebp = 0;
    // tcb[i].cont.esp = tcb[i].stackTop;// 未完成
    tcb[i].cont.ebx = 0;
    tcb[i].cont.edx = 0;
    tcb[i].cont.ecx = 0;
    tcb[i].cont.eax = 0;
    tcb[i].cont.eip = (uint32_t)start_routine;

    // 用户栈区是数组stack模拟的，最大值为MAX_STACK_SIZE
    tcb[i].stack[MAX_STACK_SIZE - 1] = (uint32_t)tcb[i].pthArg; // 参数在栈的最低点，栈底
    tcb[i].cont.esp = (uint32_t)&(tcb[i].stack[MAX_STACK_SIZE - 2]);
    tcb[i].cont.ebp = (uint32_t)&(tcb[i].stack[MAX_STACK_SIZE - 2]);
    tcb[i].retPoint = tcb[i].cont.eip;
    /*
    _________
    |__arg__|
    |_______| <- ebp esp
    |_______|
    |       |
      *****
      
    这是栈区目前的划分
    而在之后的调用中，进入一个函数的时候，比如ping_thread_function
    开头的指令为
    push %ebp
    movl %esp, %ebp
    所以此时的栈空间为
    _________
    |__arg__|
    |_______| 
    |_ebp旧值| <- ebp esp
    |       |
      *****
    所以此时arg的位置为8(%esp)，为一般函数调用的第一参数的位置

    */
    
    tcb[i].state = STATE_RUNNABLE;
    // asm volatile("movl %0, %%esp"::"m"(tcb[i].stackTop));
    // asm volatile()
    
    // 初始化线程的上下文（寄存器和堆栈）伪装成函数调用或其他类型的跳转

    return 0;
}

void pthread_exit(void *retval)
{
    tcb[current].state = STATE_DEAD;
    for (int i = 0; i < MAX_TCB_NUM; ++i)
    {
        if (tcb[i].state == STATE_BLOCKED && tcb[i].joinid == tcb[current].pthid)
        {
            tcb[i].state = STATE_RUNNABLE; // 解开锁
        }
    }
    // printf("exit %d\n", current);
    change_pthread(CHANGE_GETONE);
    return;
}

int pthread_join(pthread_t thread, void **retval)
{
    // retval 是一个无用的变量
    if (tcb[thread].state == STATE_DEAD) // 此时需要等待的线程已经DONE了
    {
        // 直接执行就可以了
    }
    else
    {
        tcb[current].joinid = thread;
        tcb[current].state = STATE_BLOCKED;
        change_pthread(CHANGE_EXCHANGE);
    }
    
    return 0;
}

int pthread_yield(void)
{
    tcb[current].state = STATE_RUNNABLE;
    change_pthread(CHANGE_EXCHANGE);
    return 0;
}

void change_pthread(int mode)
{
    // 此时是需要切换进程，这里我们就需要首先保存我们需要的内容，选择线程，之后进行还原，最后进行jump
    int i = (current + 1) % MAX_TCB_NUM;
    if (mode == CHANGE_EXCHANGE)
    {
        asm volatile("movl %%eax, %0":"=m"(tcb[current].cont.eax));
        asm volatile("movl %%ecx, %0":"=m"(tcb[current].cont.ecx));
        asm volatile("movl %%edx, %0":"=m"(tcb[current].cont.edx));
        asm volatile("movl %%ebx, %0":"=m"(tcb[current].cont.ebx));
        // asm volatile("movl %%esp, %0":"=m"(tcb[current].cont.esp));
        // asm volatile("movl %%ebp, %0":"=m"(tcb[current].cont.ebp));
        asm volatile("movl %%esi, %0":"=m"(tcb[current].cont.esi));
        asm volatile("movl %%edi, %0":"=m"(tcb[current].cont.edi));
        // asm volatile("movl %%eip, %0"::"m"(tcb[current].cont.eip));
        // 保存了可以直接保存的寄存器
        /*
        需要分析一下栈区的排布

        _________
        |_______| 上个函数的栈顶esp
        |返回地址| 
        |_ebp旧值| <- ebp
        |       |
        *****
        */
        asm volatile("leal 8(%%ebp), %0":"=a"(tcb[current].cont.esp)); // esp的位置是地址
        asm volatile("movl 4(%%ebp), %0":"=a"(tcb[current].cont.eip)); // 返回地址是需要取到数据
        asm volatile("movl (%%ebp), %0":"=a"(tcb[current].cont.ebp)); // 取出ebp的旧值
        // 以上就保存了所有的需要的恢复的信息
        // 下面需要进行线程的选择和切换
        // tcb[current].state = STATE_RUNNABLE;
    }
    for (; i != current; ++i)
    {
        if (i >= MAX_TCB_NUM) // 此时需要进行模运算
            i = i % MAX_TCB_NUM;
        
        if (tcb[i].state == STATE_RUNNABLE)
            break;
    }
    current = i; // 与之前的timerHandle不一样，这里是一定需要切换的，所以就不需要进行判断了
    tcb[current].state = STATE_RUNNING;
    // 这里已经切换完了线程

    // 下面需要进行线程的还原
    asm volatile("movl %0, %%eax"::"m"(tcb[current].cont.eax));
    asm volatile("movl %0, %%ecx"::"m"(tcb[current].cont.ecx));
    asm volatile("movl %0, %%edx"::"m"(tcb[current].cont.edx));
    asm volatile("movl %0, %%ebx"::"m"(tcb[current].cont.ebx));
    asm volatile("movl %0, %%esp"::"m"(tcb[current].cont.esp));
    asm volatile("movl %0, %%ebp"::"m"(tcb[current].cont.ebp));
    asm volatile("movl %0, %%esi"::"m"(tcb[current].cont.esi));
    asm volatile("movl %0, %%edi"::"m"(tcb[current].cont.edi));

    // asm volatile("movl %%eip, %0":"=m"(tcb[current].retPoint));
    asm volatile("jmp %0"::"m"(tcb[current].cont.eip));
    // return;
}
