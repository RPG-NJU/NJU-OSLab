#include "x86.h"
#include "device.h"

extern TSS tss;
extern ProcessTable pcb[MAX_PCB_NUM];
extern int current;

extern int displayRow;
extern int displayCol;

void GProtectFaultHandle(struct StackFrame *sf);

void timerHandle(struct StackFrame *sf);

void syscallHandle(struct StackFrame *sf);

void syscallWrite(struct StackFrame *sf);
void syscallPrint(struct StackFrame *sf);
void syscallFork(struct StackFrame *sf);
void syscallExec(struct StackFrame *sf);
void syscallSleep(struct StackFrame *sf);
void syscallExit(struct StackFrame *sf);

void kprintf(char *data, int size) 
// 自定义一个基于putChar可以在kernel中使用的printf函数
{
	for (int i = 0; i < size; ++i)
	{
		putChar(data[i]);
	}
	// putChar('\n');
	return;
}
// #define GRPIRQHANDLEDEBUG
// #define GRPIRQPRINT

void irqHandle(struct StackFrame *sf) 
{ // pointer sf = esp
	/* Reassign segment register */
	asm volatile("movw %%ax, %%ds"::"a"(KSEL(SEG_KDATA)));
	/*XXX Save esp to stackTop */
	uint32_t tmpStackTop = pcb[current].stackTop;
	pcb[current].prevStackTop = pcb[current].stackTop; // useless
	pcb[current].stackTop = (uint32_t)sf;

	switch(sf->irq) 
	{
		case -1:
			break;
		case 0xd:
		{
			// putChar('G');putChar('P');
			GProtectFaultHandle(sf);
			break;
		}
		case 0x20:
		{
			// putChar('T');
			timerHandle(sf);
			break;
		}
		case 0x80:
			syscallHandle(sf);
			break;
		default:assert(0);
	}
	/*XXX Recover stackTop */
	// putChar('1');
	pcb[current].stackTop = tmpStackTop;
}

void GProtectFaultHandle(struct StackFrame *sf) 
{
	putChar('G');
	putChar('P');
	putChar('\n');
	assert(0);
	return;
}

void timerHandle(struct StackFrame *sf) 
{
	/* echo pid of selected process */
	// putChar('0' + pcb[current].pid);
#ifdef GRPIRQHANDLEDEBUG
	kprintf("<timerHandle START>", 19); // DEBUG部分，确定进入了这个系统调用的处理函数中
	putChar('0' + pcb[current].pid);
	putChar('\n');
#endif
	// return; 
	// 如果没有定义后续的过程，则这三个语句会编译报错
	int i;
	uint32_t tmpStackTop;
	i = (current + 1) % MAX_PCB_NUM; // i指向下一个process

  // make blocked processes sleep time -1, sleep time to 0, re-run
	// HELLORPG：使拥塞进程睡眠时间 -1 一直到 0， 之后re-run
	for (int pcb_i = 0; pcb_i < MAX_PCB_NUM; ++pcb_i)
	{
		if (pcb[pcb_i].state == STATE_BLOCKED) // 被阻塞的进程
		{
			if (pcb[pcb_i].sleepTime > 0)
			{
				--pcb[pcb_i].sleepTime;
				// putChar('[');putChar('0' + pcb[pcb_i].pid);putChar(']');
			}
			if (pcb[pcb_i].sleepTime == 0)
			{
				pcb[pcb_i].state = STATE_RUNNABLE;
			}
		}
	}

  // time count not max, process continue
  // else switch to another process
	// HELLORPG：当前执行的进程的操作，如果时间片没有到达最大，则可以继续运行，如果到达最大，则考虑切换进程
	// if (pcb[current].state != STATE_RUNNING) // 此时没有运行的进程
	// {

	// }
	if ((pcb[current].state == STATE_RUNNING) && (pcb[current].timeCount < MAX_TIME_COUNT))
	{
		// putChar('?');
		putChar('0' + pcb[current].pid);
		++pcb[current].timeCount;
		return;
	}
	
	else
	{
		// putChar('!');
		pcb[current].timeCount = 0; // 进行复位
		pcb[current].state = STATE_RUNNABLE; // 变为可以运行的
		for (; i != current; ++i)
		{
			// putChar('l');
			if (i >= MAX_PCB_NUM) // 如果超出了界限，就进行模运算处理
			{
				i = i % MAX_PCB_NUM;
			}
			if (pcb[i].state == STATE_RUNNABLE && i != 0)
			{
				break;
				// current = i; // 选择了另一个进程，如果没有选择到，则仍然是原来的进程执行
			}// 个人理解是这里先不要选择，之后需要交换信息之后再进行切换
			// putChar('0' + current);
			// putChar('-');
			// putChar('0' + i);
			if (i == current)
				break;
		}
		if (current == i)
			current = 0;
		else
			current = i;
		putChar('0' + pcb[current].pid);
		pcb[i].state = STATE_RUNNING;
		// kprintf("<change to process ", 19);
		// putChar(current + '0');
		// putChar('>');
		// putChar('\n');

		// putChar('0' + (pcb[current].stackTop - (uint32_t)&(pcb[current].stackTop)));
		// putChar('\n');
		tmpStackTop = pcb[current].stackTop;
		// if (current != 0)
		// {
		tss.esp0 = pcb[current].prevStackTop;
			// tss.ss0;
		//}
		pcb[current].stackTop = pcb[current].prevStackTop;
		
		asm volatile("movl %0, %%esp"::"m"(tmpStackTop));
		// if (current == 2)
		// putChar('0' + pcb[current].pid);
	}
	
	
	
	// putChar('0' + current);
	/*XXX recover stackTop of selected process */
	
	// setting tss for user process
	// 为用户进程设置TSS寄存器
	// tss为一个全局变量
	// 如何判定是用户进程？
	// if (pcb[current].pid != 0) // 此时就一定不是内核进程
	// {
	// 	kprintf("K to U", 6);
	// 	tmpStackTop = tss.ss0 + tss.esp0;
	// 	asm volatile("movl %%esp, %0"::"m"(tmpStackTop));
	// 	asm volatile("pushl %ss");
	// 	asm volatile("pushl %esp");
	// }

	
	// switch kernel stack
	// 切换内核堆栈
		
	asm volatile("popl %gs");
	asm volatile("popl %fs");
	asm volatile("popl %es");
	asm volatile("popl %ds");
	asm volatile("popal");
	asm volatile("addl $8, %esp");
	
	asm volatile("iret");
	
}

void syscallHandle(struct StackFrame *sf) {
	switch(sf->eax) { // syscall number
		case 0:
			syscallWrite(sf);
			break; // for SYS_WRITE
		case 1:
			syscallFork(sf);
			break; // for SYS_FORK
		case 2:
			syscallExec(sf);
			break; // for SYS_EXEC
		case 3:
			syscallSleep(sf);
			break; // for SYS_SLEEP
		case 4:
			syscallExit(sf);
			break; // for SYS_EXIT
		default:break;
	}
}

void syscallWrite(struct StackFrame *sf) 
{
	switch(sf->ecx) 
	{ // file descriptor
		case 0:
			syscallPrint(sf);
			break; // for STD_OUT
		default: break;
	}
}

void syscallPrint(struct StackFrame *sf) 
{
	int sel = sf->ds; //TODO segment selector for user data, need further modification
	char *str = (char*)sf->edx;
	int size = sf->ebx;
	int i = 0;
	int pos = 0;
	char character = 0;
	uint16_t data = 0;
	asm volatile("movw %0, %%es"::"m"(sel));
	for (i = 0; i < size; i++) {
		asm volatile("movb %%es:(%1), %0":"=r"(character):"r"(str+i));
		if(character == '\n') {
			displayRow++;
			displayCol=0;
			if(displayRow==25){
				displayRow=24;
				displayCol=0;
				scrollScreen();
			}
		}
		else {
			data = character | (0x0c << 8);
			pos = (80*displayRow+displayCol)*2;
			asm volatile("movw %0, (%1)"::"r"(data),"r"(pos+0xb8000));
			displayCol++;
			if(displayCol==80){
				displayRow++;
				displayCol=0;
				if(displayRow==25){
					displayRow=24;
					displayCol=0;
					scrollScreen();
				}
			}
		}
		//asm volatile("int $0x20"); //XXX Testing irqTimer during syscall
		//asm volatile("int $0x20":::"memory"); //XXX Testing irqTimer during syscall
	}
	
	updateCursor(displayRow, displayCol);
	//TODO take care of return value
	return;
}

void syscallFork(struct StackFrame *sf) 
{
	// find empty pcb
	// 找到一个空余的PCB
	int i, j;
	for (i = 0; i < MAX_PCB_NUM; i++) 
	{
		if (pcb[i].state == STATE_DEAD)
			break;
	}
	// goto test;
	if (i != MAX_PCB_NUM) // 此时找到了空余的PCB 索引为i
	{
		/*XXX copy userspace
		  XXX enable interrupt
		 */
		enableInterrupt();
		for (j = 0; j < 0x100000; j++) 
		{
			*(uint8_t *)(j + (i+1)*0x100000) = *(uint8_t *)(j + (current+1)*0x100000); // 一个区域为0x100000
			// asm volatile("int $0x20"); //XXX Testing irqTimer during syscall
		}
		/*XXX disable interrupt
		 */
		disableInterrupt();

		/*XXX set pcb
		  XXX pcb[i]=pcb[current] doesn't work
		*/
		/*XXX set regs */
		// HELLORPG：需要设置pcb[i]

		pcb[i].pid=i;
		pcb[i].prevStackTop=pcb[current].prevStackTop+(uint32_t)&pcb[i]-(uint32_t)&pcb[current];
		pcb[i].sleepTime=0;
		for(j=0;j<MAX_STACK_SIZE;j++)
			pcb[i].stack[j]=pcb[current].stack[j];
		pcb[i].stackTop=pcb[current].stackTop + (uint32_t)&pcb[i]-(uint32_t)&pcb[current];



		pcb[i].state = STATE_RUNNABLE;
		pcb[i].timeCount = 0;
		pcb[i].sleepTime = 0;
		pcb[i].pid = i;


		// pcb[i].regs = pcb[current].regs; // 据说这种方式是不可以的
		pcb[i].regs.edi = sf->edi;//pcb[current].regs.edi;
		pcb[i].regs.esi = sf->esi;//pcb[current].regs.esi;
		pcb[i].regs.ebp = sf->ebp;//pcb[current].regs.ebp;
		pcb[i].regs.xxx = sf->xxx;//pcb[current].regs.xxx;
		pcb[i].regs.ebx = sf->ebx;//pcb[current].regs.ebx;
		pcb[i].regs.edx = sf->edx;//pcb[current].regs.edx;
		pcb[i].regs.ecx = sf->ecx;//pcb[current].regs.ecx;
		pcb[i].regs.eax = sf->eax;//pcb[current].regs.eax;
		pcb[i].regs.irq = sf->irq;//pcb[current].regs.irq;
		pcb[i].regs.error = sf->error;//pcb[current].regs.error;
		pcb[i].regs.eip = sf->eip;//pcb[current].regs.eip;
		pcb[i].regs.esp = sf->esp;//pcb[current].regs.esp;
		pcb[i].regs.eflags = sf->eflags;//pcb[current].regs.eflags;
		// // pcb[i].regs.esp = pcb[current].regs.esp;// + (i - current) * 0x100000;

		// // asm volatile("pushfl");
		// // asm volatile("popl %0":"=r"(pcb[i].regs.eflags));
		// // pcb[i].regs.eflags = sf->eflags;//pcb[i].regs.eflags;// | 0x200;
		
		pcb[i].regs.cs = USEL((1 + i * 2));
		pcb[i].regs.ss = USEL((2 + i * 2));
		// // pcb[i].regs.eip = sf->eip + (i - current) * 0x100000;
		// // putChar('<');
		// // putChar((pcb[current].regs.ecx == sf->ecx) + '0');
		// // putChar('>');putChar('\n');
		// // 经过验证，是相等的
		// // putChar('!');
		pcb[i].regs.ds = USEL((2 + i * 2));
		pcb[i].regs.es = USEL((2 + i * 2));
		pcb[i].regs.fs = USEL((2 + i * 2));
		pcb[i].regs.gs = USEL((2 + i * 2));
		
		// test:
		pcb[current].state = STATE_RUNNABLE;
		// tss.esp0 = pcb[i].stackTop;
		/*XXX set return value */
		// putChar('?');
		
		pcb[i].regs.eax = 0; // 子进程
		pcb[current].regs.eax = i; // 父进程	

		
		// pcb[i].regs.eax = 0;
		// pcb[current].regs.eax = i;

		// 切换进程
		// current = i;
		// kprintf("<fork change to process ", 19);
		// putChar(current + '0');
		// putChar('>');
		// putChar('\n');

		// // putChar('0' + (pcb[current].stackTop - (uint32_t)&(pcb[current].stackTop)));
		// // putChar('\n');
		// uint32_t tmpStackTop;
		// tmpStackTop = pcb[current].stackTop;
		// if (current != 0)
		// {
		// 	tss.esp0 = pcb[current].prevStackTop;
		// 	// tss.ss0;
		// }
		// pcb[current].stackTop = pcb[current].prevStackTop;
		
		// asm volatile("movl %0, %%esp"::"m"(tmpStackTop));
		// enableInterrupt();
		// putChar('C');
		// asm volatile("int $0x20");
	}
	else 
	{
		// test:
		pcb[current].regs.eax = -1; // 此时是fork失败
	}
	// putChar('?');
	// enableInterrupt();
	// while(1);
	return;
}

void syscallExec(struct StackFrame *sf) 
{
	return;
}

void syscallSleep(struct StackFrame *sf) 
/*
int sleep(uint32_t time)
syscall(SYS_SLEEP, (uint32_t)time, 0, 0, 0, 0)
time作为第二参数，传入ecx中
*/
{
#ifdef GRPIRQPRINT
	kprintf("<Sleep START>\n", 14);
#endif
	int i = (current + 1) % MAX_PCB_NUM;
	uint32_t tmpStackTop;

	pcb[current].state = STATE_BLOCKED;
	pcb[current].sleepTime += sf->ecx;

	for (; i != current; ++i)
	{
		if (i >= MAX_PCB_NUM) // 如果超出了界限，就进行模运算处理
		{
			i = i % MAX_PCB_NUM;
		}
		if (pcb[i].state == STATE_RUNNABLE && i != 0)
		{
			break;
		}// 个人理解是这里先不要选择，之后需要交换信息之后再进行切换
	}// 总有一个系统程序是可以执行的，所以不需要担心没有结果
	if (current == i)
		current = 0;
	else
		current = i;
	putChar('0' + pcb[current].pid);
	pcb[current].state = STATE_RUNNING;
	tmpStackTop = pcb[current].stackTop;
	tss.esp0 = pcb[current].prevStackTop;
	pcb[current].stackTop = pcb[current].prevStackTop;
	asm volatile("movl %0, %%esp"::"m"(tmpStackTop));
	/*XXX recover stackTop of selected process */
	
	// switch kernel stack
	// 切换内核堆栈		
	asm volatile("popl %gs");
	asm volatile("popl %fs");
	asm volatile("popl %es");
	asm volatile("popl %ds");
	asm volatile("popal");
	asm volatile("addl $8, %esp");
	
	asm volatile("iret");

	return;
}

void syscallExit(struct StackFrame *sf) 
{
#ifdef GRPIRQPRINT
	kprintf("<EXIT>\n", 7);
#endif
	int i = (current + 1) % MAX_PCB_NUM;
	uint32_t tmpStackTop;

	pcb[current].state = STATE_DEAD;

	for (; i != current; ++i)
	{
		if (i >= MAX_PCB_NUM) // 如果超出了界限，就进行模运算处理
		{
			i = i % MAX_PCB_NUM;
		}
		if (pcb[i].state == STATE_RUNNABLE && i != 0)
		{
			break;
		}// 个人理解是这里先不要选择，之后需要交换信息之后再进行切换
	}// 总有一个系统程序是可以执行的，所以不需要担心没有结果
	if (current == i)
		current = 0;
	else
		current = i;
	putChar('0' + pcb[current].pid);
	pcb[i].state = STATE_RUNNING;
	tmpStackTop = pcb[current].stackTop;
	tss.esp0 = pcb[current].prevStackTop;
	pcb[current].stackTop = pcb[current].prevStackTop;
	asm volatile("movl %0, %%esp"::"m"(tmpStackTop));
	/*XXX recover stackTop of selected process */
	
	// switch kernel stack
	// 切换内核堆栈		
	asm volatile("popl %gs");
	asm volatile("popl %fs");
	asm volatile("popl %es");
	asm volatile("popl %ds");
	asm volatile("popal");
	asm volatile("addl $8, %esp");
	
	asm volatile("iret");
	return;
}
