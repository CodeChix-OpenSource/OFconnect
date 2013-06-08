/*-----------------------------------------------------------------------------*/
/* Copyright: CodeChix Bay Area Chapter 2013                                   */
/*-----------------------------------------------------------------------------*/
#ifndef CC_OF_CONN_H
#define CC_OF_CONN_H

#include "cc_net_global.h"

typedef struct cc_of_chann_key_ {
    uint64_t dp_id;
    uint8_t  aux_id;
} cc_of_key_t;

typedef enum cc_of_chann_state_ {
    CC_OF_CHANNEL_DOWN = 0,
    CC_OF_CHANNEL_UP
} cc_of_state_e

typedef struct cc_of_chann_server_ {
    cc_of_key_t key;
    addr_type_e ip_ver;
    L4_type_e   layer4_proto;

    server_conn *s_conn;

    int         rw_sock_fd;

} cc_of_server;

typedef struct cc_of_client {
    cc_of_key_t  key;
    addr_type_e  ip_ver;
    L4_type_e    layer4_proto;

    client_conn  *c_conn;

    int          rw_sock_fd;
    
}cc_of_client;

#endif //CC_OF_CONN_H
