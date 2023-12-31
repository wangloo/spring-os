/*
 * Based on linux/arch/arm64/lib/bitops.h which in turn is
 * Based on arch/arm/lib/bitops.h
 *
 * Copyright (C) 2013 ARM Ltd.
 * Copyright (C) 2018 Min Le (lemin9538@gmail.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * x0: bits 4:0  bit offset
 *     bits 31:5 word offset
 * x1: address
 */

#include <asm/asm_macro.S.h>

	.section .text, "ax"

	.global test_bit

	.macro	bitop, name, instr
.global \name
\name:
.func	\name
//.cfi_startproc
	and	w3, w0, #31		// Get bit offset
	eor	w0, w0, w3		// Clear low bits
	mov	x2, #1
	add	x1, x1, x0, lsr #3	// Get word offset
	lsl	x3, x2, x3		// Create mask
1:	ldxr	w2, [x1]
	\instr	w2, w2, w3
	stxr	w0, w2, [x1]
	cbnz	w0, 1b
	ret
.endfunc
//.cfi_endproc
	.endm

	.macro	testop, name, instr
.global \name
\name:
.func	\name
//.cfi_startproc
	and	w3, w0, #31		// Get bit offset
	eor	w0, w0, w3		// Clear low bits
	mov	x2, #1
	add	x1, x1, x0, lsr #3	// Get word offset
	lsl	x4, x2, x3		// Create mask
1:	ldxr	w2, [x1]
	lsr	w0, w2, w3		// Save old value of bit
	\instr	w2, w2, w4		// toggle bit
	stlxr	w5, w2, [x1]
	cbnz	w5, 1b
	dmb	ish
	and	w0, w0, #1
3:	ret
.endfunc
//.cfi_endproc
	.endm

/*
 * Atomic bit operations.
 */
	bitop	change_bit, eor
	bitop	clear_bit, bic
	bitop	set_bit, orr

	testop	test_and_change_bit, eor
	testop	test_and_clear_bit, bic
	testop	test_and_set_bit, orr

func test_bit
	and	w3, w0, #31		// Get bit offset
	eor	w0, w0, w3		// Clear low bits
	add	x1, x1, x0, lsr #3	// Get word offset
1:	ldxr	w2, [x1]
	lsr	w0, w2, w3		// Save old value of bit
	and	w0, w0, #1
	ret
endfunc test_bit
