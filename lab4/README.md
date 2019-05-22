# OS Lab 4

## 结构体定义

[memory.h](./kernel/include/x86/memory.h)中定义了信号量`Semaphore`的结构体，已有的结构已经足够完成本次的实验：
```c
struct Semaphore {
	int state;
	int value;
	struct ListHead pcb; // link to all pcb ListHead blocked on this semaphore
};
typedef struct Semaphore Semaphore;
```
在使用的时候可以直接使用`Semaphore`进行变量的定义
`pcb`变量连接的是所有的阻塞在这个信号量上的进程的进程控制块

`Device`结构是关于标准输入输出设备的，可以不用了解



## 调用分析

[syscall.c](./lib/syscall.c)中定义了系统调用函数`syscall`
函数的声明为：
```c
int32_t syscall(int num, uint32_t a1,uint32_t a2,
		uint32_t a3, uint32_t a4, uint32_t a5);
```
可以看到，有参数的传递，除了第一个`num`是调用号之外，后续的参数为系统调用所需要的参数
对于参数的传递，有如下的映射：
```wiki
num -> eax
a1 -> ecx
a2 -> edx
a3 -> ebx
a4 -> esi
a5 -> edi
```

之后就调用[irqHandle.c](./kernel/kernel/irqHandle.c)中的中断处理函数，进入内核态
传入参数`sf`的结构定义如下：
```c
struct StackFrame {
	uint32_t gs, fs, es, ds;
	uint32_t edi, esi, ebp, xxx, ebx, edx, ecx, eax;
	uint32_t irq, error;
	uint32_t eip, cs, eflags, esp, ss;
};
```
如果为`0x80`的调用，则进一步调用`syscallHandle`函数，根据`eax`进行不同的系统调用
