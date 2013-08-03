/*-----------------------------------------------------------------------------*/
/* Copyright: CodeChix Bay Area Chapter 2013                                   */
/*-----------------------------------------------------------------------------*/
#ifndef CC_NET_CONN_H
#define CC_NET_CONN_H

typedef enum of_drv_type_ {
    CLIENT = 0,
    SERVER,
    MAX_OF_DRV_TYPE
} of_drv_type_e;

typedef enum L4_type_ {
    /* used for array index */
    TCP = 0,
    TLS,
    UDP,
    DTLS,
    /* additional types here */ 
    MAX_L4_TYPE
} L4_type_e;

typedef enum addr_type_ {
    IPV4,
    IPV6
} addr_type_e;

typedef uint32_t ipaddr_v4_t;
typedef ipv4_addr_t ipaddr_v4v6_t;

typedef struct net_svcs_ {
    int (*open_clientfd)(char *ipaddr, int port);
    int (*open_serverfd)(char *ipaddr, int port);
    int (*accept_conn)(int listenfd, struct sockaddr  *clientaddr, 
	               int *addrlen);
    int (*close_conn)(int *sockfd);
    int (*read_data)(int sockfd, void *buf, size_t len, int flags,
	             struct sockaddr *src_addr, socklen_t *addrlen);
    int (*write_data)(int sockfd, const void *buf, size_t len, int flags,
	              const struct sockaddr *dest_addr, socklen_t addrlen);
} net_svcs_t;
   
#endif //CC_NET_CONN_H
