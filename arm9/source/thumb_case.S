/* Copyright 2014 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Thumb mode toolchain helpers for compact switch/case statement.
 */

// https://chromium.googlesource.com/chromiumos/platform/ec/+/refs/heads/master/core/cortex-m0/thumb_case.S

.text
.syntax unified
.code 16
/*
 * Helpers for compact switch
 *
 * r0: the table index
 * lr: the table base address (need to clear bit 0)
 *
 * r0 and lr must be PRESERVED.
 * r12 can be clobbered.
 */
.section .text.__gnu_thumb1_case_uqi
.global __gnu_thumb1_case_uqi
.thumb_func
__gnu_thumb1_case_uqi:
	mov     r12, r1
	mov     r1, lr
	lsrs    r1, r1, #1
	lsls    r1, r1, #1
	ldrb    r1, [r1, r0]
	lsls    r1, r1, #1
	add     lr, lr, r1
	mov     r1, r12
	bx      lr
