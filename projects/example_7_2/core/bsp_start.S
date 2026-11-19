/*
 * bsp_start.S  --  Computer System Comprehensive Design, Chapter 5
 *
 * Assembly implementation of  1 + 3 + 5 + 7 + ... = n^2  with overflow pre-check.
 * Code taken from Experiment Guide section 5.2, step 8, and placed in the
 * BSP start area ("red box" of Figure 5.2).
 *
 * Overflow strategy (guide 5.2 step 9): before every addition, subtract the
 * running sum from the largest 32-bit unsigned value (t2 = 0xFFFFFFFF) and
 * compare the remaining space (t4) with the next odd number to be added (t1).
 * If t1 > t4 the addition would overflow -> stop and store the result.
 *
 * Result is written to main memory address 0x80030000.
 */

#include "regdef.h"

        .text
        .globl  asm_sum_odd
        .ent    asm_sum_odd
asm_sum_odd:

        .set    reorder
        li      t0, 0                   /* t0: accumulated sum            */
        li      t1, 1                   /* t1: current odd number         */
        li      t2, -1                  /* t2: 0xFFFFFFFF = max unsigned  */
        li      t3, 0                   /* t3: last odd number added      */
cal_loop:
        subu    t4, t2, t0              /* t4: remaining space            */
        bltu    t4, t1, save_result     /* not enough room -> save        */
        addu    t0, t0, t1              /* sum += odd number              */
        move    t3, t1                  /* archive the valid addend       */
        addu    t1, t1, 2               /* next odd number                */
        b       cal_loop
        nop
save_result:
        lui     t5, 0x8003              /* t5 = 0x80030000                */
        sw      t0, 0(t5)               /* store the sum                  */
        .set    noreorder

        jr      ra
        nop
        .end    asm_sum_odd
