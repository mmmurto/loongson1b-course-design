/*
 * example_7_2  --  Computer System Comprehensive Design, Chapter 5
 * Assembly code implementation: 1+3+5+7+... = n^2 with overflow protection.
 *
 * The assembly routine lives in core/bsp_start.S (asm_sum_odd).
 * It stores the final sum at memory address 0x80030000.
 */

#include <stdio.h>

#include "ls1b.h"
#include "mips.h"
#include "bsp.h"

extern void asm_sum_odd(void);

int main(void)
{
    printk("\r\nmain() function.\r\n");

    ls1x_drv_init();                    /* Initialize device drivers */
    install_3th_libraries();            /* Install 3th libraies      */

    asm_sum_odd();                      /* run the assembly experiment */

    printk("sum stored at 0x80030000 = 0x%08X (%u)\r\n",
           *(volatile unsigned int *)0x80030000,
           *(volatile unsigned int *)0x80030000);

    for (;;)
    {
        delay_ms(1000);
    }

    return 0;
}

/*
 * @@ End
 */
