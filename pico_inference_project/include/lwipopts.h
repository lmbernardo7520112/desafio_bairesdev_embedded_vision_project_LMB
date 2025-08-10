#ifndef __LWIPOPTS_H__
#define __LWIPOPTS_H__

/*
 * lwipopts.h - Config otimizada para uso com API RAW (udp_pcb)
 * Foco: UDP rápido, sem sockets BSD, economia de RAM.
 */

#define NO_SYS                          1   /* Usar sem sistema/RTOS */
#define SYS_LIGHTWEIGHT_PROT            0

/* Core features */
#define LWIP_RAW                        1
#define LWIP_UDP                        1
#define LWIP_TCP                        0
#define LWIP_NETCONN                    0
#define LWIP_SOCKET                     0

/* Memory */
#define MEM_ALIGNMENT                   4
#define MEM_SIZE                        (12 * 1024)

#define MEMP_NUM_PBUF                   16
#define MEMP_NUM_UDP_PCB                6
#define MEMP_NUM_SYS_TIMEOUT            8

/* PBUF pool */
#define PBUF_POOL_SIZE                  12
#define PBUF_POOL_BUFSIZE               1536

/* IPv4/ARP */
#define LWIP_IPV4                       1
#define LWIP_ARP                        1

/* Checksums */
#define CHECKSUM_GEN_IP                 1
#define CHECKSUM_GEN_UDP                1
#define CHECKSUM_CHECK_IP               1
#define CHECKSUM_CHECK_UDP              1

/* DNS/ICMP */
#define LWIP_DNS                        0
#define LWIP_ICMP                       1

/* Debug off */
#define LWIP_DEBUG                      0
#define TCPIP_DEBUG                     0
#define UDP_DEBUG                       0

/* --- Compatibilidade com LWIP_PBUF_CUSTOM_DATA_INIT --- */
/* Se a macro não estiver definida pelo SDK, cria uma versão vazia */
#ifndef LWIP_PBUF_CUSTOM_DATA_INIT
#define LWIP_PBUF_CUSTOM_DATA_INIT(p)   do {} while(0)
#endif

#endif /* __LWIPOPTS_H__ */

