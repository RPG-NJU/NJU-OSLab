.code32



.global start
start:
	# TODO

	# pushl $33 # $13
	# pushl $app_message
	# calll app_displayStr

	# 72;101;108;108;111;32;87;111;114;108;100;33;32;91;102;114;111;109;32;97;112;112;46;115;32;98;121;32;71;82;80;93
	pushl %edi
	pushl %eax
	pushl %ecx
	pushl %ebx
	

	pushl $33
	pushl $app_message

	calll app_displayStr

/*
	movl $((80*4+0)*2), %edi                # 在第5行第0列打印
	movb $0x0c, %ah                         # 黑底红字
	movl $33, %ecx
	movl (%esp), %ebx
	# movb $72, %al                           # 42为H的ASCII码
	# movw %ax, %gs:(%edi)                    # 写显存

my_loop:     
	movb $0x0c, %ah 
	movb (%ebx), %al                        
	# movb $101, %al                           
	movw %ax, %gs:(%edi)
	addl $2, %edi
	incl %ebx 
	loopnz my_loop                        

*/
	popl %ebx
	popl %ebx
	popl %ebx
	popl %ecx
	popl %eax
	popl %edi
# loop32:
	# jmp loop32
	
	ret

app_message:
	.string "Hello, World! [from app.s by GRP]\n\0" # 33

app_displayStr:
	movl 4(%esp), %ebx
	movl 8(%esp), %ecx
	movl $((80*4+0)*2), %edi
	movb $0x0c, %ah
app_nextChar:
	movb (%ebx), %al
	movw %ax, %gs:(%edi)
	addl $2, %edi
	incl %ebx
	loopnz app_nextChar # loopnz decrease ecx by 1
	ret


# displayStr:
# 	movl 4(%esp), %ebx
# 	movl 8(%esp), %ecx
# 	movl $((80*5+0)*2), %edi
# 	movb $0x0c, %ah