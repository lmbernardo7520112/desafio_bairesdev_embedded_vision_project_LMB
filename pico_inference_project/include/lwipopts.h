#ifndef LWIP_LWIPOPTS_H
#define LWIP_LWIPOPTS_H

// Enable IPv4, disable IPv6
#define LWIP_IPV4                  1
#define LWIP_IPV6                  0

// Enable essential features for Wi-Fi
#define LWIP_DHCP                  1
#define LWIP_DNS                   1
#define LWIP_ARP                   1
#define LWIP_ICMP                  1
#define LWIP_UDP                   1
#define LWIP_TCP                   1

// Disable unnecessary features
#define LWIP_IGMP                  0
#define LWIP_SNMP                  0
#define LWIP_MDNS_RESPONDER        0
#define LWIP_NETBIOS_RESPOND_NAME_QUERY 0
#define PPP_SUPPORT                0
#define LWIP_AUTOIP                0
#define LWIP_SOCKET                0
#define LWIP_NETCONN               0

// No system layer (required for pico_cyw43_arch_lwip_threadsafe_background)
#define NO_SYS                     1

// Memory configuration
#define MEM_ALIGNMENT              4
#define MEM_SIZE                   (8 * 1024)
#define PBUF_POOL_SIZE             8
#define PBUF_POOL_BUFSIZE          1536
#define MEMP_NUM_PBUF              8
#define MEMP_NUM_UDP_PCB           4
#define MEMP_NUM_TCP_PCB           2
#define MEMP_NUM_TCP_PCB_LISTEN    2
#define MEMP_NUM_TCP_SEG           8
#define MEMP_NUM_SYS_TIMEOUT       8

// TCP settings
#define TCP_MSS                    1460
#define TCP_SND_BUF                (2 * TCP_MSS)
#define TCP_WND                    (2 * TCP_MSS)
#define TCP_SND_QUEUELEN           (4 * TCP_SND_BUF / TCP_MSS)
#define TCP_SNDLOWAT               (TCP_SND_BUF / 2)
#define TCP_MAXRTX                 12
#define TCP_SYNMAXRTX              4
#define TCP_QUEUE_OOSEQ            0

// Other protocol settings
#define ARP_TABLE_SIZE             8
#define ICMP_TTL                   255
#define UDP_TTL                    255
#define TCP_TTL                    255

// Disable debugging and statistics
#define LWIP_DEBUG                 0
#define LWIP_STATS                 0

// Pico-specific settings
#define LWIP_RAND()                rand()
#define LWIP_NOASSERT              1
#define LWIP_PROVIDE_ERRNO         1

#endif /* LWIP_LWIPOPTS_H */