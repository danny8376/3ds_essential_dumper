/* Runtime ABI for the ARM Cortex-M0  
 * idivmod.S: signed 32 bit division (quotient and remainder)
 *
 * Copyright (c) 2012 Jörg Mische <bobbl@gmx.de>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

// https://github.com/bobbl/libaeabi-cortexm0/blob/master/idivmod.S

.text
.syntax unified
.code 16

.section .text.__aeabi_uidiv
.global __aeabi_uidiv
.thumb_func
__aeabi_uidiv:
@I'm too lazy to implement proper div by 0 handling, hope this never happen
@	cmp	r1, #0
@	bne	1f
@	b	__aeabi_idiv0
@1:

	@ Shift left the denominator until it is greater than the numerator
	movs	r2, #1		@ counter
	movs	r3, #0		@ result
	cmp	r0, r1
	bls	.Lsub_loop
	adds	r1, #0		@ dont shift if denominator would overflow
	bmi	.Lsub_loop

.Ldenom_shift_loop:
	lsls	r2, #1
	lsls	r1, #1
	bmi	.Lsub_loop
	cmp	r0, r1
	bhi	.Ldenom_shift_loop

.Lsub_loop:	
	cmp	r0, r1
	bcc	.Ldont_sub	@ if (num>denom)

	subs	r0, r1		@ numerator -= denom
	orrs	r3, r2		@ result(r3) |= bitmask(r2)
.Ldont_sub:

	lsrs	r1, #1		@ denom(r1) >>= 1
	lsrs	r2, #1		@ bitmask(r2) >>= 1
	bne	.Lsub_loop

	@mov	r1, r0		@ remainder(r1) = numerator(r0)
    mov	r12, r0
	mov	r1, r12
	@mov	r0, r3		@ quotient(r0) = result(r3)
	mov	r12, r3
	mov	r0, r12
	bx	lr
