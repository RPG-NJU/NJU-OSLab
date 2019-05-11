#include "common.h"
#include "x86.h"
#include "device.h"

void kEntry(void) {

	// Interruption is disabled in bootloader
	// bootloader中禁用中断

	initSerial();// initialize serial port
	// 初始化串口
	initIdt(); // initialize idt
	// 初始化IDT 中断向量表
	initIntr(); // iniialize 8259a
	// 初始化 8259a端口
	initSeg(); // initialize gdt, tss
	// 初始化 GDT TSS 寄存器
	initVga(); // initialize vga device
	// 初始化VGA设备
	initTimer(); // initialize timer device
	// 初始化时钟设备
	// putChar('R');
	initProc();
}
