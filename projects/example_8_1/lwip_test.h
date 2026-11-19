/*
 * lwip_test.h  --  Computer System Comprehensive Design, Chapter 8
 *
 * Test-mode selection, exactly as described in guide section 8.2.12 ~ 8.2.15.
 * Enable ONLY ONE macro (set the other three to 0) and rebuild.
 */

#ifndef __LWIP_TEST_H__
#define __LWIP_TEST_H__

#define  TEST_TCP_SERVER  1
#define  TEST_TCP_CLIENT  0
#define  TEST_UDP_SERVER  0
#define  TEST_UDP_CLIENT  0

/* Network parameters used by the guide (section 8.1 / 8.2 tests) */
#define  BOARD_IP0        192
#define  BOARD_IP1        168
#define  BOARD_IP2        1
#define  BOARD_IP3        123        /* Loongson 1B board address  */

#define  PC_IP0           192
#define  PC_IP1           168
#define  PC_IP2           1
#define  PC_IP3           111        /* PC / laptop address       */

#define  TCP_SERVER_PORT  9060
#define  TCP_CLIENT_PORT  9061
#define  UDP_SERVER_PORT  9062
#define  UDP_CLIENT_PORT  9063

#endif /* __LWIP_TEST_H__ */
