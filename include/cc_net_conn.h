/*-----------------------------------------------------------------------------*/
/* Copyright: CodeChix Bay Area Chapter 2013                                   */
/*-----------------------------------------------------------------------------*/
#ifndef CC_NET_CONN_H
#define CC_NET_CONN_H

#include "cc_pollthr_mgr.h"
#include "cc_of_common.h"

#define MAXBUF 65535
/* Is this optimal? */

typedef uint32_t ipaddr_v4_t;
typedef ipaddr_v4_t ipaddr_v4v6_t;

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

typedef struct cc_ofdev_key_ {
    ipaddr_v4v6_t  controller_ip_addr;
    ipaddr_v4v6_t  switch_ip_addr; 
    uint16_t       controller_L4_port;
} cc_ofdev_key_t;

/* mapping of channel key (dp-id/aux-id) to rw_sockfd) */
typedef struct cc_ofchannel_key_ {
    uint64_t  dp_id;
    uint8_t   aux_id;
} cc_ofchannel_key_t;

typedef struct cc_ofstats_ {
    uint32_t  rx_pkt;
    uint32_t  tx_pkt;
    uint32_t  tx_drops;
} cc_ofstats_t;

typedef struct cc_ofchann_info_ {
    int                   rw_sockfd;
    int                   count_retries; /* CLIENT: reconnection attempts */
    cc_ofstats_t          stats;    
} cc_ofchannel_info_t;

/* node in ofdev_htbl */
typedef struct cc_ofdev_info_ {
    cc_ofver_e     of_max_ver;  /* cc_ofver_e */
    
    GList          *ofrw_socket_list; //list of rw sockets
    GMutex	       ofrw_socket_list_lock;

    cc_of_recv_pkt       recv_func; /* cc_of_recv_pkt function ptr */

    int            main_sockfd_tcp;
    int            main_sockfd_udp;
} cc_ofdev_info_t;

typedef enum cc_ofrw_state_ {
    CC_OF_RW_DOWN = 0,
    CC_OF_RW_UP
} cc_ofrw_state_e;

typedef struct cc_ofrw_key_ {
    int       rw_sockfd;    
} cc_ofrw_key_t;

/* node in ofrw_htbl */
typedef struct cc_ofrw_info_ {
    cc_ofrw_state_e      state;
    adpoll_thread_mgr_t  *thr_mgr_p;
    cc_ofdev_key_t       dev_key;   /* needed for easier lookup of
                                       device given the rwsocket */
    L4_type_e            layer4_proto;
} cc_ofrw_info_t;

typedef struct net_svcs_ {
    int (*open_clientfd)(cc_ofdev_key_t key);
    int (*open_serverfd)(cc_ofdev_key_t key);
    int (*accept_conn)(int listenfd, cc_ofdev_key_t key);
    int (*close_conn)(int sockfd);
    ssize_t (*read_data)(int sockfd, void *buf, size_t len, int flags,
                         struct sockaddr *src_addr, socklen_t *addrlen);
    ssize_t (*write_data)(int sockfd, const void *buf, size_t len, int flags,
                          const struct sockaddr *dest_addr, socklen_t addrlen);
} net_svcs_t;
   
#endif //CC_NET_CONN_H
