#include "x86.h"
#include "device.h"

extern int displayRow;
extern int displayCol;

extern uint32_t keyBuffer[MAX_KEYBUFFER_SIZE];
extern int bufferHead;
extern int bufferTail;

void syscallHandle(struct TrapFrame *tf);
void syscallWrite(struct TrapFrame *tf);
void syscallRead(struct TrapFrame *tf);
void syscallPrint(struct TrapFrame *tf);
void syscallScan(struct TrapFrame *tf);

void GProtectFaultHandle(struct TrapFrame *tf);

void irqHandle(struct TrapFrame *tf) { // pointer tf = esp
	/*
	 * 中断处理程序
	 */
	/* Reassign segment register */
	asm volatile("movw %%ax, %%ds"::"a"(KSEL(SEG_KDATA)));
	//asm volatile("movw %%ax, %%es"::"a"(KSEL(SEG_KDATA)));
	//asm volatile("movw %%ax, %%fs"::"a"(KSEL(SEG_KDATA)));
	//asm volatile("movw %%ax, %%gs"::"a"(KSEL(SEG_KDATA)));
	switch(tf->irq) {
		case -1:
			break;
		case 0xd:
			GProtectFaultHandle(tf);
			break;
		case 0x80:
			syscallHandle(tf);
			break;
		default:assert(0);
	}
}

void syscallHandle(struct TrapFrame *tf) {
	switch(tf->eax) { // syscall number
		case 0:
			syscallWrite(tf);
			break; // for SYS_WRITE
        case 1:
            syscallRead(tf);
            break; // for SYS_READ
		default:break;
	}
}

void syscallWrite(struct TrapFrame *tf) {
	switch(tf->ecx) { // file descriptor
		case 0:
			syscallPrint(tf);
			break; // for STD_OUT
		default:break;
	}
}

void syscallRead(struct TrapFrame *tf){
    switch(tf->ecx){
        case 1:
            syscallScan(tf);
            break;
        default:break;
    }
}

void syscallPrint(struct TrapFrame *tf) {
	int sel = USEL(SEG_UDATA); //Think! segment selector for user data, need further modification
	char *str = (char*)tf->edx;
	int size = tf->ebx;
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
	}
	
	updateCursor(displayRow, displayCol);
	//Think! take care of return value
    return;
}

void syscallScan(struct TrapFrame *tf){
    // TODO: get key code by using getKeyCode and save it into keyBuffer
    uint32_t code = 0;


	code = getKeyCode();
	while (code != 0x1c && code != 0x39 && code != 0xf)
	// 0x1c为回车键，0x39为空格键，0xf为TAB键
	{
		if (code != 0)
		{

			// printf("%d\n", code); // 在kernel中无法使用自己定义的printf函数
			// if (code == 0xf)
			// 	putChar('!');
			keyBuffer[bufferTail] = code; // 将键码填入

			++bufferTail;
		}

		code = getKeyCode();
	}

	keyBuffer[bufferTail] = code; // 将使得输入过程终止的键码填入

	++bufferTail;


    // Understand this part!
    int sel = USEL(SEG_UDATA);
    char *str = (char*)tf->edx;
    int size = tf->ebx;
    int i = 0;
    char character = 0;
    asm volatile("movw %0, %%es"::"m"(sel));
    while(i < size-1){
        if(bufferHead!=bufferTail){
            character=getChar(keyBuffer[bufferHead]);
            bufferHead=(bufferHead+1)%MAX_KEYBUFFER_SIZE;
            putChar(character);
            if(character != 0){
                asm volatile("movb %0, %%es:(%1)"::"r"(character),"r"(str+i));
                i++;
            }
        }
        else
            break;
    }
    asm volatile("movb $0x00, %%es:(%0)"::"r"(str+i));
    return;
}

void GProtectFaultHandle(struct TrapFrame *tf){
	assert(0);
	return;
}
