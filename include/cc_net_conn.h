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

typdef struct ipaddr_v4v6_ {
    addr_type_e  ipaddr_type;
    union {
        uint32_t ipv4_addr;
        uint32_t ipv6_addr[4];
    } u;
} ipaddr_v4v6_t;

typedef struct net_svcs_ {
    int (*setup_conn)();
    int (*open_conn)(addr_type_e ip_ver, int *sock_fd);
    void (*close_conn)(int *sock_fd);
    int (*connect_conn)(int sock_fd);
    int (*listen_conn)();
    int (*accept_conn)();
    int (*recv_data)();
    int (*send_data)();
} net_svcs_t;

/* sample code to make this work
   of_drv_type_e s_type = CLIENT;
   L4_type_e l4_type = UDP;
   net_svcs_t *my_svcs;
   
   svcs_t CALLBACKS[MAX_SVCS_TYPE][MAX_L4_TYPE] = {
   {
   {tcp_client_open, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
   {tls_client_open, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
   {udp_client_open, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
   {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}},
   {
   {tcp_server_open, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
   {tls_server_open, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
   {udp_server_open, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
   {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}}
   };

   my_svcs = &(CALLBACKS[s_type][l4_type]);
   
   my_svcs->open();
*/
   
#endif //CC_NET_CONN_H
