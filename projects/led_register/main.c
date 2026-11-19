/*
 * led_register -- Computer System Comprehensive Design, final project
 *
 * Serial-controlled 8-bit binary LED register.
 *
 * The 8 LEDs show the value in BINARY. Commands are typed on UART3 (COM7) and
 * the board answers with DEC / HEX / BIN.
 *
 * Commands:
 *   <number>        set the register, e.g.  42   or  0x2A
 *   SET <number>    same
 *   INC             register + 1
 *   DEC             register - 1
 *   SHIFT L         shift left  (value << 1)
 *   SHIFT R         shift right (value >> 1)
 *   NOT             invert all bits
 *   RESET           register = 0
 *   BIN             print the current value only
 *   HELP            command list
 *
 * LEDs (Table 8.14): LED1..LED8 = GPIO 9,11,15,17,20,22,6,7  (active low)
 * LED1 is the LSB.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ls1b.h"
#include "mips.h"
#include "ls1b_gpio.h"
#include "ns16550.h"
#include "bsp.h"

#define GPIO_OUT 0
#define BUF_SIZE 48

static const int leds[8] = { 9, 11, 15, 17, 20, 22, 6, 7 };
static unsigned char reg_value = 0;
static char buf[BUF_SIZE];
static int  buf_len = 0;

/*----------------------------------------------------------------------------*/

static void gpio_init(void)
{
    int i;
    for (i = 0; i < 8; i++) gpio_enable(leds[i], GPIO_OUT);
}

/* LEDn = bit (n-1).  Active low: 1 -> LED on, 0 -> LED off. */
static void show_bits(unsigned char v)
{
    int i;
    for (i = 0; i < 8; i++) gpio_write(leds[i], ((v >> i) & 1) ? 0 : 1);
}

static void uart_init(void)
{
    ls1x_uart_init(devUART3, NULL);
    ls1x_uart_open(devUART3, NULL);
}

static void uart_puts(const char *s)
{
    ls1x_uart_write(devUART3, s, (int)strlen(s), NULL);
}

static void uart_putc(char c)
{
    ls1x_uart_write(devUART3, &c, 1, NULL);
}

/* decimal -> "00101010" */
static void bin_str(unsigned char v, char *out)
{
    int i;
    for (i = 0; i < 8; i++) out[i] = ((v >> (7 - i)) & 1) ? '1' : '0';
    out[8] = '\0';
}

static void report(void)
{
    char  b[9];
    char  line[80];

    bin_str(reg_value, b);
    show_bits(reg_value);
    sprintf(line, "\r\n  DEC=%3u  HEX=0x%02X  BIN=%s\r\n",
            (unsigned)reg_value, (unsigned)reg_value, b);
    uart_puts(line);
    uart_puts("  register> ");
}

static void help(void)
{
    uart_puts("\r\n--- 8-bit LED register ---\r\n");
    uart_puts("  <number> or SET <number>   set   (e.g. 42, 0x2A)\r\n");
    uart_puts("  INC / DEC                  increment / decrement\r\n");
    uart_puts("  SHIFT L / SHIFT R          shift left / right\r\n");
    uart_puts("  NOT / RESET                invert / clear\r\n");
    uart_puts("  BIN / HELP                 show value / this text\r\n");
    uart_puts("  register> ");
}

/* uppercase + trim, in place */
static void normalize(char *s)
{
    int i;
    for (i = 0; s[i]; i++)
        if (s[i] >= 'a' && s[i] <= 'z') s[i] = (char)(s[i] - 32);
}

static void handle(void)
{
    char cmd[BUF_SIZE];
    unsigned int value;

    memcpy(cmd, buf, buf_len);
    cmd[buf_len] = '\0';
    buf_len = 0;

    /* strip trailing CR/space */
    while (cmd[0] && (cmd[strlen(cmd) - 1] == '\r' ||
                      cmd[strlen(cmd) - 1] == ' '))
        cmd[strlen(cmd) - 1] = '\0';

    if (cmd[0] == '\0') { uart_puts("  register> "); return; }

    normalize(cmd);

    if (strcmp(cmd, "HELP") == 0)   { help();  return; }
    if (strcmp(cmd, "BIN")  == 0)   { report(); return; }
    if (strcmp(cmd, "RESET")== 0)   { reg_value = 0;  report(); return; }
    if (strcmp(cmd, "INC")  == 0)   { reg_value++;    report(); return; }
    if (strcmp(cmd, "DEC")  == 0)   { reg_value--;    report(); return; }
    if (strcmp(cmd, "NOT")  == 0)   { reg_value = (unsigned char)~reg_value; report(); return; }
    if (strcmp(cmd, "SHIFT L") == 0){ reg_value = (unsigned char)(reg_value << 1); report(); return; }
    if (strcmp(cmd, "SHIFT R") == 0){ reg_value = (unsigned char)(reg_value >> 1); report(); return; }

    if (strncmp(cmd, "SET ", 4) == 0) value = (unsigned int)strtoul(cmd + 4, 0, 0);
    else                              value = (unsigned int)strtoul(cmd, 0, 0);

    reg_value = (unsigned char)(value & 0xFF);
    report();
}

/*----------------------------------------------------------------------------*/

int main(void)
{
    char byte;
    char welcome[64];

    ls1x_drv_init();
    install_3th_libraries();

    gpio_init();
    uart_init();
    show_bits(0);

    bin_str(reg_value, welcome);          /* reuse buffer for the banner   */
    uart_puts("\r\n\r\n=== Loongson 1B : 8-bit LED register ===\r\n");
    uart_puts("Type HELP for the command list.\r\n");
    uart_puts("  register> ");

    for (;;)
    {
        if (ls1x_uart_read(devUART3, &byte, 1, 1) > 0)
        {
            if (byte == '\n' || byte == '\r')
            {
                handle();
            }
            else if (buf_len < BUF_SIZE - 1)
            {
                buf[buf_len++] = byte;
                uart_putc(byte);              /* local echo */
            }
        }
    }

    return 0;
}

/*
 * @@ End
 */
