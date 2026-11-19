/*
 * example_8_1  --  Computer System Comprehensive Design, Chapter 8
 *
 * Implementation of Network Transmission (lwIP 1.4.1, raw API, bare metal).
 *
 * Four mutually exclusive tests, selected in lwip_test.h:
 *   8.2.12 TEST_TCP_SERVER : board = TCP server 192.168.1.123:9060, PC = client
 *   8.2.13 TEST_TCP_CLIENT : board = TCP client -> PC server 192.168.1.111:9061
 *   8.2.14 TEST_UDP_SERVER : board = UDP server 192.168.1.123:9062, PC = client
 *   8.2.15 TEST_UDP_CLIENT : board = UDP client -> PC 192.168.1.111:9063
 *
 * The data received from the network debugging assistant (NetAssist) is printed
 * on the board's serial console (PuTTY), and echoed back to the PC.
 */

#include <stdio.h>
#include <string.h>

#include "lwip/tcp.h"
#include "lwip/udp.h"
#include "lwip/timers.h"
#include "lwip/sys.h"
#include "netif/etharp.h"

#include "ls1b.h"
#include "mips.h"
#include "bsp.h"

#include "lwip_test.h"

/*-----------------------------------------------------------------------------*/

static void show_rx(const char *proto, const void *data, unsigned int len)
{
    char msg[128];
    unsigned int n = (len < (sizeof(msg) - 1)) ? len : (sizeof(msg) - 1);

    memcpy(msg, data, n);
    msg[n] = '\0';
    printk("%s received %d bytes: %s\r\n", proto, len, msg);
}

/*-----------------------------------------------------------------------------*/
/* 8.2.12  TCP server  (board listens, PC connects as TCP Client)              */
/*-----------------------------------------------------------------------------*/
#if TEST_TCP_SERVER

static err_t tcp_srv_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
    if (p == NULL)                       /* remote host closed the connection */
    {
        tcp_close(pcb);
        return ERR_OK;
    }
    if (err != ERR_OK)
    {
        pbuf_free(p);
        return err;
    }
    show_rx("TCP server", p->payload, p->tot_len);
    tcp_write(pcb, p->payload, p->tot_len, TCP_WRITE_FLAG_COPY);   /* echo back */
    tcp_output(pcb);
    tcp_recved(pcb, p->tot_len);
    pbuf_free(p);
    return ERR_OK;
}

static err_t tcp_srv_accept(void *arg, struct tcp_pcb *newpcb, err_t err)
{
    tcp_recv(newpcb, tcp_srv_recv);
    printk("TCP server: client connected.\r\n");
    return ERR_OK;
}

static void test_thread(void)
{
    struct tcp_pcb *pcb;

    pcb = tcp_new();
    tcp_bind(pcb, IP_ADDR_ANY, TCP_SERVER_PORT);
    pcb = tcp_listen_with_backlog(pcb, 1);
    tcp_accept(pcb, tcp_srv_accept);
    printk("TCP server listening on port %d\r\n", TCP_SERVER_PORT);
}

#endif /* TEST_TCP_SERVER */

/*-----------------------------------------------------------------------------*/
/* 8.2.13  TCP client  (board connects to the PC's TCP Server)                 */
/*-----------------------------------------------------------------------------*/
#if TEST_TCP_CLIENT

static err_t tcp_cli_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
    if (p == NULL)
    {
        tcp_close(pcb);
        return ERR_OK;
    }
    show_rx("TCP client", p->payload, p->tot_len);
    tcp_write(pcb, p->payload, p->tot_len, TCP_WRITE_FLAG_COPY);
    tcp_output(pcb);
    tcp_recved(pcb, p->tot_len);
    pbuf_free(p);
    return ERR_OK;
}

static err_t tcp_cli_connected(void *arg, struct tcp_pcb *pcb, err_t err)
{
    static const char hello[] = "Loongson 1B TCP client connected.\r\n";

    printk("TCP client: connected to PC %d.%d.%d.%d:%d\r\n",
           PC_IP0, PC_IP1, PC_IP2, PC_IP3, TCP_CLIENT_PORT);
    tcp_write(pcb, hello, sizeof(hello) - 1, TCP_WRITE_FLAG_COPY);
    tcp_output(pcb);
    tcp_recv(pcb, tcp_cli_recv);
    return ERR_OK;
}

static void test_thread(void)
{
    struct tcp_pcb *pcb;
    struct ip_addr ip;
    err_t e;

    IP4_ADDR(&ip, PC_IP0, PC_IP1, PC_IP2, PC_IP3);
    pcb = tcp_new();
    e = tcp_connect(pcb, &ip, TCP_CLIENT_PORT, tcp_cli_connected);
    printk("TCP client connecting to %d.%d.%d.%d:%d (err=%d) ...\r\n",
           PC_IP0, PC_IP1, PC_IP2, PC_IP3, TCP_CLIENT_PORT, e);
}

#endif /* TEST_TCP_CLIENT */

/*-----------------------------------------------------------------------------*/
/* 8.2.14  UDP server  (board listens on 9062, PC sends as UDP)                */
/*-----------------------------------------------------------------------------*/
#if TEST_UDP_SERVER

static void udp_srv_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p,
                         struct ip_addr *addr, u16_t port)
{
    show_rx("UDP server", p->payload, p->tot_len);
    udp_sendto(pcb, p, addr, port);                    /* echo back */
    pbuf_free(p);
}

static void test_thread(void)
{
    struct udp_pcb *pcb = udp_new();

    udp_bind(pcb, IP_ADDR_ANY, UDP_SERVER_PORT);
    udp_recv(pcb, udp_srv_recv, NULL);
    printk("UDP server ready on port %d\r\n", UDP_SERVER_PORT);
}

#endif /* TEST_UDP_SERVER */

/*-----------------------------------------------------------------------------*/
/* 8.2.15  UDP client  (board sends to the PC's UDP port)                      */
/*-----------------------------------------------------------------------------*/
#if TEST_UDP_CLIENT

static void udp_cli_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p,
                         struct ip_addr *addr, u16_t port)
{
    show_rx("UDP client", p->payload, p->tot_len);
    pbuf_free(p);
}

static void test_thread(void)
{
    struct udp_pcb *pcb = udp_new();
    struct ip_addr ip;
    struct pbuf *p;
    static const char hello[] = "Loongson 1B UDP client.\r\n";

    IP4_ADDR(&ip, PC_IP0, PC_IP1, PC_IP2, PC_IP3);
    udp_bind(pcb, IP_ADDR_ANY, UDP_CLIENT_PORT);
    udp_recv(pcb, udp_cli_recv, NULL);
    udp_connect(pcb, &ip, UDP_CLIENT_PORT);

    p = pbuf_alloc(PBUF_TRANSPORT, sizeof(hello) - 1, PBUF_RAM);
    if (p != NULL)
    {
        memcpy(p->payload, hello, sizeof(hello) - 1);
        udp_send(pcb, p);
        pbuf_free(p);
    }
    printk("UDP client sending to %d.%d.%d.%d:%d\r\n",
           PC_IP0, PC_IP1, PC_IP2, PC_IP3, UDP_CLIENT_PORT);
}

#endif /* TEST_UDP_CLIENT */

/*-----------------------------------------------------------------------------*/

int main(void)
{
    printk("\r\nmain() function.  Chapter 8 - network transmission.\r\n");

    ls1x_drv_init();               /* Initialize device drivers            */
    install_3th_libraries();       /* lwIP init + GMAC0 glue (BSP_USE_GMAC0)*/

    printk("board IP = %d.%d.%d.%d\r\n", BOARD_IP0, BOARD_IP1, BOARD_IP2, BOARD_IP3);

    test_thread();                 /* start the selected test              */

    for (;;)                       /* raw API needs periodic timer service */
    {
        sys_check_timeouts();
        delay_ms(1);
    }

    return 0;
}

/*
 * @@ End
 */
