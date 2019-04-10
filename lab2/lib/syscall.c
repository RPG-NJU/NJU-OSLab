#include "lib.h"
#include "types.h"


/*
 * io lib here
 * 库函数写在这
 */

// 理论上所有的函数只需要写在这个文件中就可以了

// This stdarg.h define

#ifndef _STDARG_H_
#define _STDARG_H_

#ifndef RC_INVOKED

#ifndef _VA_LIST_DEFINED
#define _VA_LIST_DEFINED
#endif

#ifndef    _VA_LIST
#define _VA_LIST
typedef char* va_list;
#endif

#define __va_argsiz(t)    \
    (((sizeof(t) + sizeof(int) - 1) / sizeof(int)) * sizeof(int))

#ifdef    __GNUC__

#define va_start(ap, pN)    \
    ((ap) = ((va_list) __builtin_next_arg(pN)))
#else

#define va_start(ap, pN)    \
    ((ap) = ((va_list) (&pN) + __va_argsiz(pN)))
#endif

#define va_end(ap)    ((void)0)

#define va_arg(ap, t)                    \
     (((ap) = (ap) + __va_argsiz(t)),        \
      *((t*) (void*) ((ap) - __va_argsiz(t))))

#endif

#endif

// End of stdarg.h

/* 
以上的资料以及相关的宏解析来自网络，参考如下：
fenxiduiyuif ifif;
为部分的参考资料的来源，但是并未列全，可能有遗漏
 */
// 对于stdarg.h的适当解析：
/*
stdarg.h头文件定义了一个变量类型va_list和三个宏，用来在参数个数位置的函数中获取参数
可变参数的函数通过在参数列表末尾使用省略号来定义

三个宏分别为：
va_start
va_arg
va_end

对于他们的解析如下：
va_start(va_list ap, last_arg)
其中，ap为需要初始化的变量，而last_arg为最后一个传递给函数的已知固定参数，也就是省略号之前的一个参数

type va_arg(va_list ap, type)
这个宏检索ap参数列表中的下一个类型为type的参数

void va_end(va_list ap)
这个宏允许使用了va_start宏的带有可变参数的函数返回，如果在从函数返回之前没有调用这个宏，则结果为未定义

 */


//static inline int32_t syscall(int num, uint32_t a1,uint32_t a2,
int32_t syscall(int num, uint32_t a1,uint32_t a2,
		uint32_t a3, uint32_t a4, uint32_t a5) //这部分的函数不需要自己定义，已经完成了相应的寄存器的传值以及INT 0x80操作
{
	int32_t ret = 0;
	//Generic system call: pass system call number in AX
	//up to five parameters in DX,CX,BX,DI,SI
	//Interrupt kernel with T_SYSCALL
	//
	//The "volatile" tells the assembler not to optimize
	//this instruction away just because we don't use the
	//return value
	//
	//The last clause tells the assembler that this can potentially
	//change the condition and arbitrary memory locations.

	/*
	XXX Note: ebp shouldn't be flushed
	    May not be necessary to store the value of eax, ebx, ecx, edx, esi, edi
	*/
	uint32_t eax, ecx, edx, ebx, esi, edi;
	uint16_t selector;
	
	asm volatile("movl %%eax, %0":"=m"(eax));
	asm volatile("movl %%ecx, %0":"=m"(ecx));
	asm volatile("movl %%edx, %0":"=m"(edx));
	asm volatile("movl %%ebx, %0":"=m"(ebx));
	asm volatile("movl %%esi, %0":"=m"(esi));
	asm volatile("movl %%edi, %0":"=m"(edi));
	asm volatile("movl %0, %%eax"::"m"(num));
	asm volatile("movl %0, %%ecx"::"m"(a1));
	asm volatile("movl %0, %%edx"::"m"(a2));
	asm volatile("movl %0, %%ebx"::"m"(a3));
	asm volatile("movl %0, %%esi"::"m"(a4));
	asm volatile("movl %0, %%edi"::"m"(a5));
	asm volatile("int $0x80");
	asm volatile("movl %%eax, %0":"=m"(ret));
	asm volatile("movl %0, %%eax"::"m"(eax));
	asm volatile("movl %0, %%ecx"::"m"(ecx));
	asm volatile("movl %0, %%edx"::"m"(edx));
	asm volatile("movl %0, %%ebx"::"m"(ebx));
	asm volatile("movl %0, %%esi"::"m"(esi));
	asm volatile("movl %0, %%edi"::"m"(edi));
	
	asm volatile("movw %%ss, %0":"=m"(selector)); //XXX %ds is reset after iret
	//selector = 16;
	asm volatile("movw %%ax, %%ds"::"a"(selector));
	
	return ret;
}

// API to support format in printf, if you can't understand, use your own implementation!
int dec2Str(int decimal, char *buffer, int size, int count);
int hex2Str(uint32_t hexadecimal, char *buffer, int size, int count);
int str2Str(char *string, char *buffer, int size, int count);

int printd(int output, char *buffer, int size, int count); // 期望返回的应该是count的新值
int printc(char output, char *buffer, int size, int count);
int prints(char *output, char *buffer, int size, int count);
int printx(int output, char *buffer, int size, int count);
int print_default(char c /*这个是导致非法的format中的字符，也要输出*/, char *buffer, int size, int count);


int printf(const char *format,...) // 第一个参数固定为format格式参数，只需要对这个参数进行讨论即可
{
	int i=0; // format index
	char buffer[MAX_BUFFER_SIZE];
	int count=0; // buffer index
	// int index=0; // parameter index
	// void *paraList=(void*)&format; // address of format in stack
	// int state=0; // 0: legal character; 1: '%'; 2: illegal format
	// int decimal=0;
	// uint32_t hexadecimal=0;
	// char *string=0;
	// char character=0;
	va_list ap;
	va_start(ap, format);
	// 初始化va_list数据结构

	while(format[i] != 0)
	{
        // TODO: support more format %s %d %x and so on
		if (format[i] == '%') // 此时是格式符的开头，需要考虑下一个位置的字符，进行swtich
		{
			++i; // 对format[i]之后的下一个字符进行判定

			switch(format[i])
			{
				case 'd':
				{
					count = printd(va_arg(ap, int), buffer, MAX_BUFFER_SIZE, count);
					//++i; // 对format格式符的下一个字符进行讨论
				}break;
				case 'c':
				{
					count = printc(va_arg(ap, char), buffer, MAX_BUFFER_SIZE, count);
					//++i;
				}break;
				case 's':
				{
					count = prints(va_arg(ap, char*), buffer, MAX_BUFFER_SIZE, count);
					//++i;
				}break;
				case 'x':
				{
					count = printx(va_arg(ap, int), buffer, MAX_BUFFER_SIZE, count);
					//++i;
				}break;
				default: // 都不是以上的合法格式符，则应该是会输出这个%
				{
					// 既然都不是以上的格式符，说明是非法的关键字，就直接输出就可以了，仿照之前的输出，可以得到如下的代码
					count = print_default(format[i], buffer, MAX_BUFFER_SIZE, count);
					//++i;
				}break;
			}
			++i; // 集中执行一样的语句
		}
		
		else
		{
			buffer[count]=format[i];
			count++;
			if(count==MAX_BUFFER_SIZE) 
			{
				syscall(SYS_WRITE, STD_OUT, (uint32_t)buffer, (uint32_t)MAX_BUFFER_SIZE, 0, 0);
				count=0;
			}
			i++;
		}
	}
	if(count!=0)
		syscall(SYS_WRITE, STD_OUT, (uint32_t)buffer, (uint32_t)count, 0, 0);

	va_end(ap);
	// 需要调用这个宏以结束上述过程

    return 0;
}

int dec2Str(int decimal, char *buffer, int size, int count) {
	int i=0;
	int temp;
	int number[16];

	if(decimal<0){
		buffer[count]='-';
		count++;
		if(count==size) {
			syscall(SYS_WRITE, STD_OUT, (uint32_t)buffer, (uint32_t)size, 0, 0);
			count=0;
		}
		temp=decimal/10;
		number[i]=temp*10-decimal;
		decimal=temp;
		i++;
		while(decimal!=0){
			temp=decimal/10;
			number[i]=temp*10-decimal;
			decimal=temp;
			i++;
		}
	}
	else{
		temp=decimal/10;
		number[i]=decimal-temp*10;
		decimal=temp;
		i++;
		while(decimal!=0){
			temp=decimal/10;
			number[i]=decimal-temp*10;
			decimal=temp;
			i++;
		}
	}

	while(i!=0){
		buffer[count]=number[i-1]+'0';
		count++;
		if(count==size) {
			syscall(SYS_WRITE, STD_OUT, (uint32_t)buffer, (uint32_t)size, 0, 0);
			count=0;
		}
		i--;
	}
	return count;
}

int hex2Str(uint32_t hexadecimal, char *buffer, int size, int count) {
	int i=0;
	uint32_t temp=0;
	int number[16];

	temp=hexadecimal>>4;
	number[i]=hexadecimal-(temp<<4);
	hexadecimal=temp;
	i++;
	while(hexadecimal!=0){
		temp=hexadecimal>>4;
		number[i]=hexadecimal-(temp<<4);
		hexadecimal=temp;
		i++;
	}

	while(i!=0){
		if(number[i-1]<10)
			buffer[count]=number[i-1]+'0';
		else
			buffer[count]=number[i-1]-10+'a';
		count++;
		if(count==size) {
			syscall(SYS_WRITE, STD_OUT, (uint32_t)buffer, (uint32_t)size, 0, 0);
			count=0;
		}
		i--;
	}
	return count;
}

int str2Str(char *string, char *buffer, int size, int count) {
	int i=0;
	while(string[i]!=0){
		buffer[count]=string[i];
		count++;
		if(count==size) {
			syscall(SYS_WRITE, STD_OUT, (uint32_t)buffer, (uint32_t)size, 0, 0);
			count=0;
		}
		i++;
	}
	return count;
}

int printd(int output, char *buffer, int size, int count)
{
	return dec2Str(output, buffer, size, count);
}

int printc(char output, char *buffer, int size, int count)
{
	buffer[count] = output; // 将需要输出的字符加入buffer缓冲区等待输出
	++count; // 计数

	if (count == size) // 如果此时已经突破了缓冲区的最大size
	{
		syscall(SYS_WRITE, STD_OUT, (uint32_t)buffer, (uint32_t)MAX_BUFFER_SIZE, 0, 0);
		count = 0;
	}
	return count;
}

int prints(char *output, char *buffer, int size, int count)
{
	return str2Str(output, buffer, size, count);
}

int printx(int output, char *buffer, int size, int count)
{
	return hex2Str(output, buffer, size, count);
}

int print_default(char c /*这个是导致非法的format中的字符，也要输出*/, char *buffer, int size, int count)
{
    // 需要注意，经过测试，在标准库的实现中，如果在format中需要输出%，则需要“%%”才能输出一个百分号
	// 但是实际情况又比较复杂，在实验报告中会有相信的叙述，但是在我实现的代码中，做出如下的规定：
	// 当format字符串中出现了%但是后续的字符不构成合理有效的格式符，则直接忽略掉最初的一个%，转而输出后续的一个字符
	buffer[count] = c;
	++count;
	if (count == size)
	{
		syscall(SYS_WRITE, STD_OUT, (uint32_t)buffer, (uint32_t)size, 0, 0);
		count = 0;
	}
	// 之后让%之后的非法的字符进入缓冲区

	return count;
}

// API to support format in scanf, if you can't understand, use your own implementation!
int matchWhiteSpace(char *buffer, int size, int *count);
int str2Dec(int *dec, char *buffer, int size, int *count);
int str2Hex(int *hex, char *buffer, int size, int *count);
int str2Str2(char *string, int avail, char *buffer, int size, int *count);

int scanfd(int *input, char *buffer, int size, int count);
// 希望返回的仍然是count，但是不确定能否成功
int scanfs(char *input, char *buffer, int size, int count);
int scanfc(char *input, char *buffer, int size, int count);
int scanfx(int *input, char *buffer, int size, int count);
int scanf_default(const char *format, char *buffer, int format_begin, int size, int count);
int stoi(const char *str, int *num, int begin); // 返回值为索引值
int scanfns(char *input, char *buffer, int size, int count, int len);

int scanf(const char *format,...) 
{
    // TODO: implement scanf function, return the number of input parameters
	// 返回值为输入的参数的个数
	// 参考了网上的教程：https://www.cnblogs.com/cpoint/p/3373263.html
	// 但是实际上审视了助教提供的API之后可以发现，很多的输入都不需要自己再进行考量了
	// 基本上只有c和num s这两种格式符需要有一定的代码量，而其他的就只需要进行适当的调用就可以了

	va_list ap;
	va_start(ap, format);
	// 同样，对不定参数的数据结构进行初始化
	
	int parameters_num = 0; // 记录输入了几个参数
	int buffer_count = 0; // 与前面的实现类似，是对于buffer的一个index访问

	int i = 0;

	char buffer[MAX_BUFFER_SIZE]; // 用于存储输入的buffer缓冲区
	buffer[0] = 0; // 进行初始化，因为后续的API中需要对缓冲区的第一个字节有要求，必须为0

	// int number_head_index = 0; // 用于表示当前数字的第一个索引

	while(format[i] != 0) // 只要不是结束符，就可以继续的读取
	{
		if (format[i] == '%') // 有可能是格式符
		{
			++i; // add index
			
			switch(format[i])
			{
				case 'd':
				{
					int *input = va_arg(ap, int*);
					buffer_count = scanfd(input, buffer, MAX_BUFFER_SIZE, buffer_count);
					++parameters_num; // 已经输入的参数+1
					buffer[0] = 0; // 这个是给输入API所提供的条件，否则可能会导致无法正确使用API
					++i;
				}break;
				case 'c':
				{
					
					// if (syscall(SYS_READ, STD_IN, (uint32_t)buffer, (uint32_t)MAX_BUFFER_SIZE, 0, 0) == 1) // 如果顺利从STD读入
					// {
					// 	char *input = va_arg(ap, char*);
					// 	*input = buffer[0];
					// 	++parameters_num;
					// 	buffer[0] = 0;
					// }
					// 本来打算进行这样子的编程模式，但是为了一定的模块化，最终决定进行函数调用，通过子函数的模式来完成上述的操作
					char *input = va_arg(ap, char*);
					buffer_count = scanfc(input, buffer, MAX_BUFFER_SIZE, buffer_count);
					++parameters_num;
					buffer[0] = 0;
					++i;
				}break; 
				case 's':
				{
					char *input = va_arg(ap, char*);
					buffer_count = scanfs(input, buffer, MAX_BUFFER_SIZE, buffer_count);
					++parameters_num;
					buffer[0] = 0;
					++i;
				}break;
				case 'x':
				{
					int *input = va_arg(ap, int*);
					buffer_count = scanfx(input, buffer, MAX_BUFFER_SIZE, buffer_count);
					++parameters_num;
					buffer[0] = 0;
					++i;
				}break;

				default:
				{
					// 这之中有两种可能，一种是%num s这种固定长度字符串的要求，另一种是非格式符
					if (format[i] <= '9' && format[i] >= '1') // 此时就是数字
					{
						// number_head_index = i;
						int len = 0;
						i = stoi(format, &len, i);
						if (format[i] == 's')
						{
							char *input = va_arg(ap, char*);
							buffer_count = scanfns(input, buffer, MAX_BUFFER_SIZE, buffer_count, len);
							++parameters_num;
							buffer[0] = 0;
							++i;
						}
					}
					else
					{
						// i = scanf_default(format, buffer, i, MAX_BUFFER_SIZE, buffer_count);
						// if (i == -1)
						// {
						// 	va_end(ap);
						// 	return parameters_num;
						// }
						// 理应直接跳过%不作处理，而对下一个i进行处理
						// 这是经过自己在Windows10下使用VS环境的测试而确定的
					}
					
				}break;
			}
			while(format[i] == ' ')
			{
				// printf("?");
				++i;
			}
		}
		else
		{
			// printf("Hello!!!!!!\n");
			i = scanf_default(format, buffer, i, MAX_BUFFER_SIZE, buffer_count);
			if (i == -1)
			{
				va_end(ap);
				return parameters_num;
			}
		}
		
	}

	va_end(ap);


    return parameters_num;
}

int matchWhiteSpace(char *buffer, int size, int *count) // 这里表示了所有被标识为空白符的识别
{
	int ret=0;
	while(1)
	{
		if(buffer[*count]==0)
		{
			do
			{
				ret=syscall(SYS_READ, STD_IN, (uint32_t)buffer, (uint32_t)size, 0, 0);
			}while(ret == 0 || ret == -1);
			(*count)=0;
		}
		if(buffer[*count]==' ' ||
		   buffer[*count]=='\t' ||
		   buffer[*count]=='\n'){
			(*count)++;
		}
		else
			return 0;
	}
}

int str2Dec(int *dec, char *buffer, int size, int *count) {
	int sign=0; // positive integer
	int state=0;
	int integer=0;
	int ret=0;
	while(1){
		if(buffer[*count]==0){
			do{
				ret=syscall(SYS_READ, STD_IN, (uint32_t)buffer, (uint32_t)size, 0, 0);
			}while(ret == 0 || ret == -1);
			(*count)=0;
		}
		if(state==0){
			if(buffer[*count]=='-'){
				state=1;
				sign=1;
				(*count)++;
			}
			else if(buffer[*count]>='0' &&
				buffer[*count]<='9'){
				state=2;
				integer=buffer[*count]-'0';
				(*count)++;
			}
			else if(buffer[*count]==' ' ||
				buffer[*count]=='\t' ||
				buffer[*count]=='\n'){
				state=0;
				(*count)++;
			}
			else
				return -1;
		}
		else if(state==1){
			if(buffer[*count]>='0' &&
			   buffer[*count]<='9'){
				state=2;
				integer=buffer[*count]-'0';
				(*count)++;
			}
			else
				return -1;
		}
		else if(state==2){
			if(buffer[*count]>='0' &&
			   buffer[*count]<='9'){
				state=2;
				integer*=10;
				integer+=buffer[*count]-'0';
				(*count)++;
			}
			else{
				if(sign==1)
					*dec=-integer;
				else
					*dec=integer;
				return 0;
			}
		}
		else
			return -1;
	}
	return 0;
}

int str2Hex(int *hex, char *buffer, int size, int *count) {
	int state=0;
	int integer=0;
	int ret=0;
	while(1){
		if(buffer[*count]==0){
			do{
				ret=syscall(SYS_READ, STD_IN, (uint32_t)buffer, (uint32_t)size, 0, 0);
			}while(ret == 0 || ret == -1);
			(*count)=0;
		}
		if(state==0){
			if(buffer[*count]=='0'){
				state=1;
				(*count)++;
			}
			else if(buffer[*count]==' ' ||
				buffer[*count]=='\t' ||
				buffer[*count]=='\n'){
				state=0;
				(*count)++;
			}
			else
				return -1;
		}
		else if(state==1){
			if(buffer[*count]=='x'){
				state=2;
				(*count)++;
			}
			else
				return -1;
		}
		else if(state==2){
			if(buffer[*count]>='0' && buffer[*count]<='9'){
				state=3;
				integer*=16;
				integer+=buffer[*count]-'0';
				(*count)++;
			}
			else if(buffer[*count]>='a' && buffer[*count]<='f'){
				state=3;
				integer*=16;
				integer+=buffer[*count]-'a'+10;
				(*count)++;
			}
			else if(buffer[*count]>='A' && buffer[*count]<='F'){
				state=3;
				integer*=16;
				integer+=buffer[*count]-'A'+10;
				(*count)++;
			}
			else
				return -1;
		}
		else if(state==3){
			if(buffer[*count]>='0' && buffer[*count]<='9'){
				state=3;
				integer*=16;
				integer+=buffer[*count]-'0';
				(*count)++;
			}
			else if(buffer[*count]>='a' && buffer[*count]<='f'){
				state=3;
				integer*=16;
				integer+=buffer[*count]-'a'+10;
				(*count)++;
			}
			else if(buffer[*count]>='A' && buffer[*count]<='F'){
				state=3;
				integer*=16;
				integer+=buffer[*count]-'A'+10;
				(*count)++;
			}
			else{
				*hex=integer;
				return 0;
			}
		}
		else
			return -1;
	}
	return 0;
}

int str2Str2(char *string, int avail, char *buffer, int size, int *count) {
	int i=0;
	int state=0;
	int ret=0;
	while(i < avail-1){
		if(buffer[*count]==0){
			do{
				ret=syscall(SYS_READ, STD_IN, (uint32_t)buffer, (uint32_t)size, 0, 0);
			}while(ret == 0 || ret == -1);
			(*count)=0;
		}
		if(state==0){
			if(buffer[*count]==' ' ||
			   buffer[*count]=='\t' ||
			   buffer[*count]=='\n'){
				state=0;
				(*count)++;
			}
			else{
				state=1;
				string[i]=buffer[*count];
				i++;
				(*count)++;
			}
		}
		else if(state==1){
			if(buffer[*count]==' ' ||
			   buffer[*count]=='\t' ||
			   buffer[*count]=='\n'){
				string[i]=0;
				return 0;
			}
			else{
				state=1;
				string[i]=buffer[*count];
				i++;
				(*count)++;
			}
		}
		else
			return -1;
	}
	string[i]=0;
	return 0;
}

int scanfd(int *input, char *buffer, int size, int count)
{
	str2Dec(input, buffer, size, &count);
	// 与前面的printf在这部分的实现不同，因为返回值永远是0，所以只能通过这种方式来改变count的值
	return count;
}

int scanfs(char *input, char *buffer, int size, int count)
{
	str2Str2(input, 0x7fffffff, buffer, size, &count);

	return count;
}

int scanfc(char *input, char *buffer, int size, int count)
{
	count = 0;
	if (syscall(SYS_READ, STD_IN, (uint32_t)buffer, (uint32_t)MAX_BUFFER_SIZE, 0, 0) == 1) // 如果顺利从STD读入
	{
		*input = buffer[0];
		//++parameters_num;
	}
	return count;
}

int scanfx(int *input, char *buffer, int size, int count)
{
	str2Hex(input, buffer, size, &count);

	return count;
}

int scanf_default(const char *format, char *buffer, int format_begin, int size, int count)
{
	
	count = 0; // 归位
	buffer[0] = 0;
	if (buffer[0] == 0) // 仿照API进行书写
	{
		
		if (syscall(SYS_READ, STD_IN, (uint32_t)buffer, (uint32_t)MAX_BUFFER_SIZE, 0, 0) == 1)
		{
			//printf("%s\n", format);
			//printf("HELLO\n");
			//printf("%sw\n", buffer);
			// 只有这个时候，输出才是有效的，并且是可以读取的
			for (int i = 0; buffer[i] != 0; ++format_begin, ++i)
			{
				// printf("hello");
				if (buffer[i] != format[format_begin])
				{
					//printf("{%c}{%c}\n", buffer[i], format[format_begin]);
					buffer[0] = 0;
					return -1;
				}
			}
			buffer[0] = 0;
			return format_begin;
		}
		buffer[0] = 0;
		return 0;
	}
	buffer[0] = 0;
	return 0;
}

int stoi(const char *str, int *num, int begin)
{
	*num = 0;
	for (; str[begin] <= '9' && str[begin] >= '0'; ++begin)
	{
		*num = *num * 10 + str[begin] - '0';
	}
	return begin;
}

int scanfns(char *input, char *buffer, int size, int count, int len)
{
	str2Str2(input, len, buffer, size, &count);

	return count;
}