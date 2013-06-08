/*-----------------------------------------------------------------------------*/
/* Copyright: CodeChix Bay Area Chapter 2013                                   */
/*-----------------------------------------------------------------------------*/
#ifndef CC_NET_CONN_H
#define CC_NET_CONN_H

typedef enum L4_type_ {
    /* used for array index */
    TCP = 0,
//    TLS,
//    UDP,
//    DTLS,
    /* additional types here */ 
    MAX_L4_TYPE
} L4_type_e;

typedef enum conn_type_ {
    CLIENT = 0,
    SERVER,
    MAX_CONN_TYPE
} conn_type_e;

typedef enum addr_type_ {
    IPV4,
    IPV6
} addr_type_e;

typedef struct client_svcs_ {
    int (*open_conn)(addr_type_e ip_ver, int *sock_fd);
    void (*close_conn)(int *sock_fd);
    int (*connect_conn)(int sock_fd);
    int (*recv_data)();
    int (*send_data)();
} client_svcs_t;

typedef struct server_svcs_ {
    int (*open_conn)(addr_type_e ip_ver, int *sock_fd);
    void (*close_conn)(int *sock_fd);
    int (*listen_conn)();
    int (*accept_conn)();
    int (*recv_data)();
    int (*send_data)();
} server_svcs_t;

typedef struct conn_key_ {
    int sockfd;
} conn_key_t;


/* A single server connection with multiple threads
 * to manage the r/w sockets
 */
typedef struct server_conn_ {
    conn_key_t      key;
    server_svcs_t   *svcs;

    addr_type_e   ip_ver;
    int           listen_sockfd;

    uint_32       rw_socket_thread_count;
    
    /*list of rw_sock_fd */
    GList         *rw_sockfd_list;
            
    /* thread information */
    GList         *rw_thread_list;
    
    /* state */

    /* stats */

} server_conn;

/* One client connection per r/w socket for which a thread
 * will be spawned
 */
typedef struct client_conn_ {
    conn_key_t        key;
    client_svcs_t    *svcs;

    addr_type_e   ip_ver;

    int           rw_sockfd;
    
    /* thread information */
    

    /* state */
    
    /* stats */
} client_conn;


#endif //CC_NET_CONN_H
