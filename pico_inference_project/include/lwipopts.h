// lwipopts.h - Configuração otimizada para UDP rápido no Raspberry Pi Pico W
// Leonardo - 2025-08-08

#ifndef LWIPOPTS_H
#define LWIPOPTS_H

// --------------------------------------------------------------------
// Configuração básica
// --------------------------------------------------------------------
#define NO_SYS                      1   // Sem RTOS (uso bare-metal)
#define LWIP_SOCKET                 0   // Sem API de sockets (usa raw API)
#define LWIP_NETCONN                0   // Desativa netconn API (menos RAM)

// --------------------------------------------------------------------
// UDP
// --------------------------------------------------------------------
#define LWIP_UDP                    1
#define LWIP_UDPLITE                0
#define UDP_TTL                     255
#define MEMP_NUM_UDP_PCB            4   // Número de conexões UDP simultâneas

// --------------------------------------------------------------------
// TCP (desativado para economizar RAM)
// --------------------------------------------------------------------
#define LWIP_TCP                    0
#define LWIP_LISTEN_BACKLOG         0

// --------------------------------------------------------------------
// IPv4 e IPv6
// --------------------------------------------------------------------
#define LWIP_IPV4                   1
#define LWIP_IPV6                   0

// --------------------------------------------------------------------
// Memória e buffers (ajustado para Pico W - 264KB RAM total)
// --------------------------------------------------------------------
#define MEM_ALIGNMENT               4
#define MEM_SIZE                    (8 * 1024)   // Heap do lwIP: 8KB
#define MEMP_NUM_PBUF               8
#define PBUF_POOL_SIZE              8
#define PBUF_POOL_BUFSIZE           512          // Tamanho do buffer de pool
#define LWIP_RAM_HEAP_POINTER       NULL         // Usa malloc padrão do SDK

// --------------------------------------------------------------------
// Checksum (feito por hardware no CYW43? Não, no Pico W é por software)
// --------------------------------------------------------------------
#define CHECKSUM_GEN_IP             1
#define CHECKSUM_GEN_UDP            1
#define CHECKSUM_CHECK_IP           1
#define CHECKSUM_CHECK_UDP          1

// --------------------------------------------------------------------
// ARP e DHCP
// --------------------------------------------------------------------
#define LWIP_ARP                    1
#define ARP_TABLE_SIZE              4
#define ARP_QUEUEING                0
#define LWIP_DHCP                   1
#define LWIP_AUTOIP                 0
#define LWIP_DHCP_DOES_ACD_CHECK    0

// --------------------------------------------------------------------
// Timeouts
// --------------------------------------------------------------------
#define LWIP_TCPIP_TIMEOUT          1
#define LWIP_SO_RCVTIMEO            0
#define LWIP_SO_SNDTIMEO            0

// --------------------------------------------------------------------
// Debug (desative para máxima performance)
// --------------------------------------------------------------------
#define LWIP_DEBUG                  0

#endif // LWIPOPTS_H

