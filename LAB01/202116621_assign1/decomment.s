	.file	"decomment.c"
	.text
	.section	.rodata
.LC0:
	.string	""
.LC1:
	.string	"Locale Setting failed"
.LC2:
	.string	"./src/decomment.c"
.LC3:
	.string	"0"
	.text
	.globl	main
	.type	main, @function
main:
.LFB0:
	.cfi_startproc
	endbr64
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movq	%fs:40, %rax
	movq	%rax, -8(%rbp)
	xorl	%eax, %eax
	leaq	.LC0(%rip), %rax
	movq	%rax, %rsi
	movl	$0, %edi
	call	setlocale@PLT
	testq	%rax, %rax
	jne	.L2
	movq	stderr(%rip), %rax
	movq	%rax, %rcx
	movl	$21, %edx
	movl	$1, %esi
	leaq	.LC1(%rip), %rax
	movq	%rax, %rdi
	call	fwrite@PLT
	movl	$1, %eax
	jmp	.L18
.L2:
	movl	$1, -28(%rbp)
	movl	$-1, -24(%rbp)
	movl	$0, -20(%rbp)
.L17:
	call	getchar@PLT
	movl	%eax, -16(%rbp)
	cmpl	$-1, -16(%rbp)
	jne	.L4
	leaq	-24(%rbp), %rdx
	leaq	-20(%rbp), %rax
	movq	%rdx, %rsi
	movq	%rax, %rdi
	call	handleEOF
	movl	%eax, -12(%rbp)
	movl	-12(%rbp), %eax
	jmp	.L18
.L4:
	movl	-16(%rbp), %eax
	movb	%al, -29(%rbp)
	movl	-20(%rbp), %eax
	cmpl	$8, %eax
	ja	.L5
	movl	%eax, %eax
	leaq	0(,%rax,4), %rdx
	leaq	.L7(%rip), %rax
	movl	(%rdx,%rax), %eax
	cltq
	leaq	.L7(%rip), %rdx
	addq	%rdx, %rax
	notrack jmp	*%rax
	.section	.rodata
	.align 4
	.align 4
.L7:
	.long	.L15-.L7
	.long	.L14-.L7
	.long	.L13-.L7
	.long	.L12-.L7
	.long	.L11-.L7
	.long	.L10-.L7
	.long	.L9-.L7
	.long	.L8-.L7
	.long	.L6-.L7
	.text
.L15:
	movsbl	-29(%rbp), %eax
	leaq	-28(%rbp), %rdx
	leaq	-20(%rbp), %rcx
	movq	%rcx, %rsi
	movl	%eax, %edi
	call	handleNormal
	jmp	.L16
.L14:
	movsbl	-29(%rbp), %eax
	leaq	-24(%rbp), %rcx
	leaq	-28(%rbp), %rdx
	leaq	-20(%rbp), %rsi
	movl	%eax, %edi
	call	handleSlash
	jmp	.L16
.L13:
	movsbl	-29(%rbp), %eax
	leaq	-28(%rbp), %rdx
	leaq	-20(%rbp), %rcx
	movq	%rcx, %rsi
	movl	%eax, %edi
	call	handleSingleCom
	jmp	.L16
.L12:
	movsbl	-29(%rbp), %eax
	leaq	-24(%rbp), %rcx
	leaq	-28(%rbp), %rdx
	leaq	-20(%rbp), %rsi
	movl	%eax, %edi
	call	handleMultiCom
	jmp	.L16
.L11:
	movsbl	-29(%rbp), %eax
	leaq	-24(%rbp), %rcx
	leaq	-28(%rbp), %rdx
	leaq	-20(%rbp), %rsi
	movl	%eax, %edi
	call	handleAfterStar
	jmp	.L16
.L10:
	movsbl	-29(%rbp), %eax
	leaq	-28(%rbp), %rdx
	leaq	-20(%rbp), %rcx
	movq	%rcx, %rsi
	movl	%eax, %edi
	call	handleString
	jmp	.L16
.L9:
	movsbl	-29(%rbp), %eax
	leaq	-28(%rbp), %rdx
	leaq	-20(%rbp), %rcx
	movq	%rcx, %rsi
	movl	%eax, %edi
	call	handleStringEsc
	jmp	.L16
.L8:
	movsbl	-29(%rbp), %eax
	leaq	-28(%rbp), %rdx
	leaq	-20(%rbp), %rcx
	movq	%rcx, %rsi
	movl	%eax, %edi
	call	handleChar
	jmp	.L16
.L6:
	movsbl	-29(%rbp), %eax
	leaq	-28(%rbp), %rdx
	leaq	-20(%rbp), %rcx
	movq	%rcx, %rsi
	movl	%eax, %edi
	call	handleCharEsc
	jmp	.L16
.L5:
	leaq	__PRETTY_FUNCTION__.0(%rip), %rax
	movq	%rax, %rcx
	movl	$103, %edx
	leaq	.LC2(%rip), %rax
	movq	%rax, %rsi
	leaq	.LC3(%rip), %rax
	movq	%rax, %rdi
	call	__assert_fail@PLT
.L16:
	jmp	.L17
.L18:
	movq	-8(%rbp), %rdx
	subq	%fs:40, %rdx
	je	.L19
	call	__stack_chk_fail@PLT
.L19:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE0:
	.size	main, .-main
	.section	.rodata
	.align 8
.LC4:
	.string	"Error: line %d: unterminated comment\n"
	.text
	.globl	handleEOF
	.type	handleEOF, @function
handleEOF:
.LFB1:
	.cfi_startproc
	endbr64
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movq	%rdi, -8(%rbp)
	movq	%rsi, -16(%rbp)
	movq	-16(%rbp), %rdx
	movq	-8(%rbp), %rax
	movq	%rax, %rsi
	movl	$2, %edi
	movl	$0, %eax
	call	checkPointers
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	cmpl	$3, %eax
	jne	.L21
	movq	-16(%rbp), %rax
	movl	(%rax), %edx
	movq	stderr(%rip), %rax
	leaq	.LC4(%rip), %rcx
	movq	%rcx, %rsi
	movq	%rax, %rdi
	movl	$0, %eax
	call	fprintf@PLT
	movl	$1, %eax
	jmp	.L22
.L21:
	movl	$0, %eax
.L22:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE1:
	.size	handleEOF, .-handleEOF
	.globl	handleNormal
	.type	handleNormal, @function
handleNormal:
.LFB2:
	.cfi_startproc
	endbr64
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movl	%edi, %eax
	movq	%rsi, -16(%rbp)
	movq	%rdx, -24(%rbp)
	movb	%al, -4(%rbp)
	movq	-24(%rbp), %rdx
	movq	-16(%rbp), %rax
	movq	%rax, %rsi
	movl	$2, %edi
	movl	$0, %eax
	call	checkPointers
	movsbl	-4(%rbp), %eax
	cmpl	$47, %eax
	je	.L24
	cmpl	$47, %eax
	jg	.L25
	cmpl	$39, %eax
	je	.L26
	cmpl	$39, %eax
	jg	.L25
	cmpl	$10, %eax
	je	.L27
	cmpl	$34, %eax
	je	.L28
	jmp	.L25
.L24:
	movq	-16(%rbp), %rax
	movl	$1, (%rax)
	jmp	.L29
.L28:
	movq	-16(%rbp), %rax
	movl	$5, (%rax)
	movsbl	-4(%rbp), %eax
	movq	stdout(%rip), %rdx
	movq	%rdx, %rsi
	movl	%eax, %edi
	call	fputc@PLT
	jmp	.L29
.L26:
	movq	-16(%rbp), %rax
	movl	$7, (%rax)
	movsbl	-4(%rbp), %eax
	movq	stdout(%rip), %rdx
	movq	%rdx, %rsi
	movl	%eax, %edi
	call	fputc@PLT
	jmp	.L29
.L27:
	movsbl	-4(%rbp), %eax
	movq	stdout(%rip), %rdx
	movq	%rdx, %rsi
	movl	%eax, %edi
	call	fputc@PLT
	movq	-24(%rbp), %rax
	movl	(%rax), %eax
	leal	1(%rax), %edx
	movq	-24(%rbp), %rax
	movl	%edx, (%rax)
	jmp	.L29
.L25:
	movsbl	-4(%rbp), %eax
	movq	stdout(%rip), %rdx
	movq	%rdx, %rsi
	movl	%eax, %edi
	call	fputc@PLT
	nop
.L29:
	nop
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE2:
	.size	handleNormal, .-handleNormal
	.globl	handleSlash
	.type	handleSlash, @function
handleSlash:
.LFB3:
	.cfi_startproc
	endbr64
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movl	%edi, %eax
	movq	%rsi, -16(%rbp)
	movq	%rdx, -24(%rbp)
	movq	%rcx, -32(%rbp)
	movb	%al, -4(%rbp)
	movq	-32(%rbp), %rcx
	movq	-24(%rbp), %rdx
	movq	-16(%rbp), %rax
	movq	%rax, %rsi
	movl	$3, %edi
	movl	$0, %eax
	call	checkPointers
	movsbl	-4(%rbp), %eax
	subl	$10, %eax
	cmpl	$37, %eax
	ja	.L31
	movl	%eax, %eax
	leaq	0(,%rax,4), %rdx
	leaq	.L33(%rip), %rax
	movl	(%rdx,%rax), %eax
	cltq
	leaq	.L33(%rip), %rdx
	addq	%rdx, %rax
	notrack jmp	*%rax
	.section	.rodata
	.align 4
	.align 4
.L33:
	.long	.L37-.L33
	.long	.L31-.L33
	.long	.L31-.L33
	.long	.L31-.L33
	.long	.L31-.L33
	.long	.L31-.L33
	.long	.L31-.L33
	.long	.L31-.L33
	.long	.L31-.L33
	.long	.L31-.L33
	.long	.L31-.L33
	.long	.L31-.L33
	.long	.L31-.L33
	.long	.L31-.L33
	.long	.L31-.L33
	.long	.L31-.L33
	.long	.L31-.L33
	.long	.L31-.L33
	.long	.L31-.L33
	.long	.L31-.L33
	.long	.L31-.L33
	.long	.L31-.L33
	.long	.L31-.L33
	.long	.L31-.L33
	.long	.L36-.L33
	.long	.L31-.L33
	.long	.L31-.L33
	.long	.L31-.L33
	.long	.L31-.L33
	.long	.L35-.L33
	.long	.L31-.L33
	.long	.L31-.L33
	.long	.L34-.L33
	.long	.L31-.L33
	.long	.L31-.L33
	.long	.L31-.L33
	.long	.L31-.L33
	.long	.L32-.L33
	.text
.L32:
	movq	-16(%rbp), %rax
	movl	$2, (%rax)
	movq	-24(%rbp), %rax
	movl	(%rax), %edx
	movq	-32(%rbp), %rax
	movl	%edx, (%rax)
	movq	stdout(%rip), %rax
	movq	%rax, %rsi
	movl	$32, %edi
	call	fputc@PLT
	jmp	.L38
.L34:
	movq	-16(%rbp), %rax
	movl	$3, (%rax)
	movq	stdout(%rip), %rax
	movq	%rax, %rsi
	movl	$32, %edi
	call	fputc@PLT
	movq	-24(%rbp), %rax
	movl	(%rax), %edx
	movq	-32(%rbp), %rax
	movl	%edx, (%rax)
	jmp	.L38
.L36:
	movq	-16(%rbp), %rax
	movl	$5, (%rax)
	movq	stdout(%rip), %rax
	movq	%rax, %rsi
	movl	$47, %edi
	call	fputc@PLT
	movsbl	-4(%rbp), %eax
	movq	stdout(%rip), %rdx
	movq	%rdx, %rsi
	movl	%eax, %edi
	call	fputc@PLT
	jmp	.L38
.L35:
	movq	-16(%rbp), %rax
	movl	$7, (%rax)
	movq	stdout(%rip), %rax
	movq	%rax, %rsi
	movl	$47, %edi
	call	fputc@PLT
	movsbl	-4(%rbp), %eax
	movq	stdout(%rip), %rdx
	movq	%rdx, %rsi
	movl	%eax, %edi
	call	fputc@PLT
	jmp	.L38
.L37:
	movq	-16(%rbp), %rax
	movl	$0, (%rax)
	movq	stdout(%rip), %rax
	movq	%rax, %rsi
	movl	$47, %edi
	call	fputc@PLT
	movsbl	-4(%rbp), %eax
	movq	stdout(%rip), %rdx
	movq	%rdx, %rsi
	movl	%eax, %edi
	call	fputc@PLT
	movq	-24(%rbp), %rax
	movl	(%rax), %eax
	leal	1(%rax), %edx
	movq	-24(%rbp), %rax
	movl	%edx, (%rax)
	jmp	.L38
.L31:
	movq	-16(%rbp), %rax
	movl	$0, (%rax)
	movq	stdout(%rip), %rax
	movq	%rax, %rsi
	movl	$47, %edi
	call	fputc@PLT
	movsbl	-4(%rbp), %eax
	movq	stdout(%rip), %rdx
	movq	%rdx, %rsi
	movl	%eax, %edi
	call	fputc@PLT
	nop
.L38:
	nop
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE3:
	.size	handleSlash, .-handleSlash
	.globl	handleSingleCom
	.type	handleSingleCom, @function
handleSingleCom:
.LFB4:
	.cfi_startproc
	endbr64
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movl	%edi, %eax
	movq	%rsi, -16(%rbp)
	movq	%rdx, -24(%rbp)
	movb	%al, -4(%rbp)
	movq	-24(%rbp), %rdx
	movq	-16(%rbp), %rax
	movq	%rax, %rsi
	movl	$2, %edi
	movl	$0, %eax
	call	checkPointers
	cmpb	$10, -4(%rbp)
	jne	.L41
	movq	-16(%rbp), %rax
	movl	$0, (%rax)
	movsbl	-4(%rbp), %eax
	movq	stdout(%rip), %rdx
	movq	%rdx, %rsi
	movl	%eax, %edi
	call	fputc@PLT
	movq	-24(%rbp), %rax
	movl	(%rax), %eax
	leal	1(%rax), %edx
	movq	-24(%rbp), %rax
	movl	%edx, (%rax)
.L41:
	nop
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE4:
	.size	handleSingleCom, .-handleSingleCom
	.globl	handleMultiCom
	.type	handleMultiCom, @function
handleMultiCom:
.LFB5:
	.cfi_startproc
	endbr64
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movl	%edi, %eax
	movq	%rsi, -16(%rbp)
	movq	%rdx, -24(%rbp)
	movq	%rcx, -32(%rbp)
	movb	%al, -4(%rbp)
	movq	-32(%rbp), %rcx
	movq	-24(%rbp), %rdx
	movq	-16(%rbp), %rax
	movq	%rax, %rsi
	movl	$3, %edi
	movl	$0, %eax
	call	checkPointers
	movsbl	-4(%rbp), %eax
	cmpl	$10, %eax
	je	.L43
	cmpl	$42, %eax
	jne	.L46
	movq	-16(%rbp), %rax
	movl	$4, (%rax)
	jmp	.L45
.L43:
	movsbl	-4(%rbp), %eax
	movq	stdout(%rip), %rdx
	movq	%rdx, %rsi
	movl	%eax, %edi
	call	fputc@PLT
	movq	-24(%rbp), %rax
	movl	(%rax), %eax
	leal	1(%rax), %edx
	movq	-24(%rbp), %rax
	movl	%edx, (%rax)
	jmp	.L45
.L46:
	nop
.L45:
	nop
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE5:
	.size	handleMultiCom, .-handleMultiCom
	.globl	handleAfterStar
	.type	handleAfterStar, @function
handleAfterStar:
.LFB6:
	.cfi_startproc
	endbr64
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movl	%edi, %eax
	movq	%rsi, -16(%rbp)
	movq	%rdx, -24(%rbp)
	movq	%rcx, -32(%rbp)
	movb	%al, -4(%rbp)
	movq	-32(%rbp), %rcx
	movq	-24(%rbp), %rdx
	movq	-16(%rbp), %rax
	movq	%rax, %rsi
	movl	$3, %edi
	movl	$0, %eax
	call	checkPointers
	movsbl	-4(%rbp), %eax
	cmpl	$47, %eax
	je	.L48
	cmpl	$47, %eax
	jg	.L49
	cmpl	$10, %eax
	je	.L50
	cmpl	$42, %eax
	je	.L53
	jmp	.L49
.L48:
	movq	-16(%rbp), %rax
	movl	$0, (%rax)
	jmp	.L52
.L50:
	movq	-16(%rbp), %rax
	movl	$3, (%rax)
	movsbl	-4(%rbp), %eax
	movq	stdout(%rip), %rdx
	movq	%rdx, %rsi
	movl	%eax, %edi
	call	fputc@PLT
	movq	-24(%rbp), %rax
	movl	(%rax), %eax
	leal	1(%rax), %edx
	movq	-24(%rbp), %rax
	movl	%edx, (%rax)
	jmp	.L52
.L49:
	movq	-16(%rbp), %rax
	movl	$3, (%rax)
	jmp	.L52
.L53:
	nop
.L52:
	nop
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE6:
	.size	handleAfterStar, .-handleAfterStar
	.globl	handleString
	.type	handleString, @function
handleString:
.LFB7:
	.cfi_startproc
	endbr64
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movl	%edi, %eax
	movq	%rsi, -16(%rbp)
	movq	%rdx, -24(%rbp)
	movb	%al, -4(%rbp)
	movq	-24(%rbp), %rdx
	movq	-16(%rbp), %rax
	movq	%rax, %rsi
	movl	$2, %edi
	movl	$0, %eax
	call	checkPointers
	movsbl	-4(%rbp), %eax
	cmpl	$92, %eax
	je	.L55
	cmpl	$92, %eax
	jg	.L56
	cmpl	$10, %eax
	je	.L57
	cmpl	$34, %eax
	jne	.L56
	movq	-16(%rbp), %rax
	movl	$0, (%rax)
	movsbl	-4(%rbp), %eax
	movq	stdout(%rip), %rdx
	movq	%rdx, %rsi
	movl	%eax, %edi
	call	fputc@PLT
	jmp	.L58
.L57:
	movsbl	-4(%rbp), %eax
	movq	stdout(%rip), %rdx
	movq	%rdx, %rsi
	movl	%eax, %edi
	call	fputc@PLT
	movq	-24(%rbp), %rax
	movl	(%rax), %eax
	leal	1(%rax), %edx
	movq	-24(%rbp), %rax
	movl	%edx, (%rax)
	jmp	.L58
.L55:
	movq	-16(%rbp), %rax
	movl	$6, (%rax)
	movsbl	-4(%rbp), %eax
	movq	stdout(%rip), %rdx
	movq	%rdx, %rsi
	movl	%eax, %edi
	call	fputc@PLT
	jmp	.L58
.L56:
	movsbl	-4(%rbp), %eax
	movq	stdout(%rip), %rdx
	movq	%rdx, %rsi
	movl	%eax, %edi
	call	fputc@PLT
	nop
.L58:
	nop
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE7:
	.size	handleString, .-handleString
	.globl	handleStringEsc
	.type	handleStringEsc, @function
handleStringEsc:
.LFB8:
	.cfi_startproc
	endbr64
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movl	%edi, %eax
	movq	%rsi, -16(%rbp)
	movq	%rdx, -24(%rbp)
	movb	%al, -4(%rbp)
	movq	-24(%rbp), %rdx
	movq	-16(%rbp), %rax
	movq	%rax, %rsi
	movl	$2, %edi
	movl	$0, %eax
	call	checkPointers
	movq	-16(%rbp), %rax
	movl	$5, (%rax)
	movsbl	-4(%rbp), %eax
	movq	stdout(%rip), %rdx
	movq	%rdx, %rsi
	movl	%eax, %edi
	call	fputc@PLT
	cmpb	$10, -4(%rbp)
	jne	.L61
	movq	-24(%rbp), %rax
	movl	(%rax), %eax
	leal	1(%rax), %edx
	movq	-24(%rbp), %rax
	movl	%edx, (%rax)
.L61:
	nop
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE8:
	.size	handleStringEsc, .-handleStringEsc
	.globl	handleChar
	.type	handleChar, @function
handleChar:
.LFB9:
	.cfi_startproc
	endbr64
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movl	%edi, %eax
	movq	%rsi, -16(%rbp)
	movq	%rdx, -24(%rbp)
	movb	%al, -4(%rbp)
	movq	-24(%rbp), %rdx
	movq	-16(%rbp), %rax
	movq	%rax, %rsi
	movl	$2, %edi
	movl	$0, %eax
	call	checkPointers
	movsbl	-4(%rbp), %eax
	cmpl	$92, %eax
	je	.L63
	cmpl	$92, %eax
	jg	.L64
	cmpl	$10, %eax
	je	.L65
	cmpl	$39, %eax
	jne	.L64
	movq	-16(%rbp), %rax
	movl	$0, (%rax)
	movsbl	-4(%rbp), %eax
	movq	stdout(%rip), %rdx
	movq	%rdx, %rsi
	movl	%eax, %edi
	call	fputc@PLT
	jmp	.L66
.L65:
	movsbl	-4(%rbp), %eax
	movq	stdout(%rip), %rdx
	movq	%rdx, %rsi
	movl	%eax, %edi
	call	fputc@PLT
	movq	-24(%rbp), %rax
	movl	(%rax), %eax
	leal	1(%rax), %edx
	movq	-24(%rbp), %rax
	movl	%edx, (%rax)
	jmp	.L66
.L63:
	movq	-16(%rbp), %rax
	movl	$8, (%rax)
	movsbl	-4(%rbp), %eax
	movq	stdout(%rip), %rdx
	movq	%rdx, %rsi
	movl	%eax, %edi
	call	fputc@PLT
	jmp	.L66
.L64:
	movsbl	-4(%rbp), %eax
	movq	stdout(%rip), %rdx
	movq	%rdx, %rsi
	movl	%eax, %edi
	call	fputc@PLT
	nop
.L66:
	nop
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE9:
	.size	handleChar, .-handleChar
	.globl	handleCharEsc
	.type	handleCharEsc, @function
handleCharEsc:
.LFB10:
	.cfi_startproc
	endbr64
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movl	%edi, %eax
	movq	%rsi, -16(%rbp)
	movq	%rdx, -24(%rbp)
	movb	%al, -4(%rbp)
	movq	-24(%rbp), %rdx
	movq	-16(%rbp), %rax
	movq	%rax, %rsi
	movl	$2, %edi
	movl	$0, %eax
	call	checkPointers
	movq	-16(%rbp), %rax
	movl	$7, (%rax)
	movsbl	-4(%rbp), %eax
	movq	stdout(%rip), %rdx
	movq	%rdx, %rsi
	movl	%eax, %edi
	call	fputc@PLT
	cmpb	$10, -4(%rbp)
	jne	.L69
	movq	-24(%rbp), %rax
	movl	(%rax), %eax
	leal	1(%rax), %edx
	movq	-24(%rbp), %rax
	movl	%edx, (%rax)
.L69:
	nop
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE10:
	.size	handleCharEsc, .-handleCharEsc
	.section	.rodata
	.align 8
.LC5:
	.string	"Error: Null pointer encountered\n"
	.text
	.globl	checkPointers
	.type	checkPointers, @function
checkPointers:
.LFB11:
	.cfi_startproc
	endbr64
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$240, %rsp
	movl	%edi, -228(%rbp)
	movq	%rsi, -168(%rbp)
	movq	%rdx, -160(%rbp)
	movq	%rcx, -152(%rbp)
	movq	%r8, -144(%rbp)
	movq	%r9, -136(%rbp)
	testb	%al, %al
	je	.L71
	movaps	%xmm0, -128(%rbp)
	movaps	%xmm1, -112(%rbp)
	movaps	%xmm2, -96(%rbp)
	movaps	%xmm3, -80(%rbp)
	movaps	%xmm4, -64(%rbp)
	movaps	%xmm5, -48(%rbp)
	movaps	%xmm6, -32(%rbp)
	movaps	%xmm7, -16(%rbp)
.L71:
	movq	%fs:40, %rax
	movq	%rax, -184(%rbp)
	xorl	%eax, %eax
	movl	$8, -208(%rbp)
	movl	$48, -204(%rbp)
	leaq	16(%rbp), %rax
	movq	%rax, -200(%rbp)
	leaq	-176(%rbp), %rax
	movq	%rax, -192(%rbp)
	movl	$0, -220(%rbp)
	jmp	.L72
.L76:
	movl	-208(%rbp), %eax
	cmpl	$47, %eax
	ja	.L73
	movq	-192(%rbp), %rax
	movl	-208(%rbp), %edx
	movl	%edx, %edx
	addq	%rdx, %rax
	movl	-208(%rbp), %edx
	addl	$8, %edx
	movl	%edx, -208(%rbp)
	jmp	.L74
.L73:
	movq	-200(%rbp), %rax
	leaq	8(%rax), %rdx
	movq	%rdx, -200(%rbp)
.L74:
	movq	(%rax), %rax
	movq	%rax, -216(%rbp)
	cmpq	$0, -216(%rbp)
	jne	.L75
	movq	stderr(%rip), %rax
	movq	%rax, %rcx
	movl	$32, %edx
	movl	$1, %esi
	leaq	.LC5(%rip), %rax
	movq	%rax, %rdi
	call	fwrite@PLT
	movl	$1, %edi
	call	exit@PLT
.L75:
	addl	$1, -220(%rbp)
.L72:
	movl	-220(%rbp), %eax
	cmpl	-228(%rbp), %eax
	jl	.L76
	nop
	movq	-184(%rbp), %rax
	subq	%fs:40, %rax
	je	.L77
	call	__stack_chk_fail@PLT
.L77:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE11:
	.size	checkPointers, .-checkPointers
	.section	.rodata
	.type	__PRETTY_FUNCTION__.0, @object
	.size	__PRETTY_FUNCTION__.0, 5
__PRETTY_FUNCTION__.0:
	.string	"main"
	.ident	"GCC: (Ubuntu 11.4.0-1ubuntu1~22.04) 11.4.0"
	.section	.note.GNU-stack,"",@progbits
	.section	.note.gnu.property,"a"
	.align 8
	.long	1f - 0f
	.long	4f - 1f
	.long	5
0:
	.string	"GNU"
1:
	.align 8
	.long	0xc0000002
	.long	3f - 2f
2:
	.long	0x3
3:
	.align 8
4:
