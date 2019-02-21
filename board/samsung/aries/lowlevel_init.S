/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Memory Setup stuff - taken from blob memsetup.S
 *
 * Copyright (C) 2009 Samsung Electronics
 * Kyungmin Park <kyungmin.park@samsung.com>
 */

#include <config.h>
#include <asm/arch/cpu.h>
#include <asm/arch/clock.h>
#include <asm/arch/power.h>

/*
 * Register usages:
 *
 * r5 has zero always
 * r7 has S5PC100 GPIO base, 0xE0300000
 * r8 has real GPIO base, 0xE0300000, 0xE0200000 at S5PC100, S5PC110 repectively
 * r9 has Mobile DDR size, 1 means 1GiB, 2 means 2GiB and so on
 */

	.globl lowlevel_init
lowlevel_init:
	ldr	r8, =0xE0200000

	bl	ps_hold_reg
	bl	watchdog_disable

	/* for UART SEL */
	bl	uart_asm_init

	bl	jump_to_sgs_ibl

	/* init_system */
	bl	0xD0007CAC

	/* backup uart pointer */
	ldr	r0, =0xD0035458
	ldr	r6, [r0]

	/* clear iram */
	ldr	r0, =0xD000C90C
	ldr	r1, =0xD0035400
	mov	r2, #0x70
	bl	rebell_memcpy

	/* restore uart pointer */
	ldr	r0, =0xD0035458
	str	r6, [r0]

	/* clear usb stuff */
	ldr	r0, =0xD0035470
	mov	r1, #0
	mov	r2, #0x128
	bl	rebell_fillmem

	/* start usb booting */
	bl	0xD0007D78

	/* jump to whatever was sent via USB */
	ldr	r0, =0xD00354D0
	ldr	r0, [r0]
	bx	r0

/*
 * ps_hold_reg: Set to high to prevent turn off
 */
ps_hold_reg:
	ldr 	r1, =0xE010E81C
	ldr	r0, [r1]
	orr	r0, r0, #0x300
	orr	r0, r0, #0x1
	str	r0, [r1]
	mov	pc, lr

/*
 * watchdog_disable
 */
watchdog_disable:
	ldr	r1, =0xE2700000
	mov	r0, #0
	str	r0, [r1]
	mov	pc, lr

/*
 * uart_asm_init: Initialize UART's pins
 */
uart_asm_init:
	/*
	 * Note that the following address
	 * 0xE020'0360 is reserved address at S5PC100
	 */
	/* UART_SEL MP0_5[7] at S5PC110 */
	add	r0, r8, #0x360			@ S5PC110_GPIO_MP0_5_OFFSET
	ldr	r1, [r0, #0x0]			@ S5PC1XX_GPIO_CON_OFFSET
	bic	r1, r1, #(0xf << 28)		@ 28 = 7 * 4-bit
	orr	r1, r1, #(0x1 << 28)		@ Output
	str	r1, [r0, #0x0]			@ S5PC1XX_GPIO_CON_OFFSET

	ldr	r1, [r0, #0x8]			@ S5PC1XX_GPIO_PULL_OFFSET
	bic	r1, r1, #(0x3 << 14)		@ 14 = 7 * 2-bit
	orr	r1, r1, #(0x2 << 14)		@ Pull-up enabled
	str	r1, [r0, #0x8]			@ S5PC1XX_GPIO_PULL_OFFSET

	ldr	r1, [r0, #0x4]			@ S5PC1XX_GPIO_DAT_OFFSET
	orr	r1, r1, #(1 << 7)		@ 7 = 7 * 1-bit
	str	r1, [r0, #0x4]			@ S5PC1XX_GPIO_DAT_OFFSET
	mov	pc, lr

@void rebell_memcpy(int source(r0), int target(r1), int size(r2))
@returns num of bytes copied (always equal to size or just hangs on mem access violation)
rebell_memcpy:
	STMFD	SP!, {R3-R6,LR}

	MOV	R3, R2
	MOV	R2, R1
	MOV	R1, R0

	MOV	R0, R1
	MOV	R1, R2
	MOV	R2, R3

	MOV	R3, #0
rebell_memcpy_copyloop:
	ldrb	r4, [r0,r3] @src
	strb	r4, [r1,r3] @dst
	add	r3, r3, #1
	cmp	r3, r2
	BLT	rebell_memcpy_copyloop
	MOV	R0, R2
	LDMFD	SP!, {R3-R6,PC}

@void rebell_fillmem(R0 void* ptr, R1 byte fillbyte, R2 int length)
rebell_fillmem:
	STMFD	SP!, {R3,LR}
	MOV	R3, #0
rebell_fillmem_loop:
	CMP	R3, R2
	BGE	rebell_fillmem_ret
	STRB	R1, [R0, R3]
	ADD	R3, R3, #1
	B	rebell_fillmem_loop
rebell_fillmem_ret:
	LDMFD	SP!, {R3,PC}

/*
 * jump_to_sgs_ibl: Helper to jump to platform config from IBL
 */
jump_to_sgs_ibl:
	stmfd	sp!, {lr}
	.align	4
	.incbin	"/home/jon/android/bootloaders/hummingbird-hibl/HummingBirdInterceptorBootloader/init_by_rebell.bin",0,0x8c0
