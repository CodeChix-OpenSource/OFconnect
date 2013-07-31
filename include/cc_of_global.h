/*-----------------------------------------------------------------------------*/
/* Copyright: CodeChix Bay Area Chapter 2013                                   */
/*-----------------------------------------------------------------------------*/
#ifndef CC_OF_GLOBAL_H
#define CC_OF_GLOBAL_H

#include <glib.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "cc_net_conn.h"
#include "cc_log.h"
#include "cc_pollthr_mgr.h"

#define SIZE_RW_THREAD_BUCKET        10
#define MAX_PER_THREAD_RWSOCKETS     100

typedef enum of_dev_type_ {
    SWITCH = 0,
    CONTROLLER,
    MAX_OF_DEV_TYPE
} of_dev_type_e;

typedef enum cc_ofrw_state_ {
    CC_OF_RW_DOWN = 0,
    CC_OF_RW_UP
} cc_ofrw_state_e;

typdef struct cc_ofrw_key_ {
    int       rw_sockfd;    
} cc_ofrw_key_t;

typedef struct cc_ofstats_ {
    uint32_t  rx_pkt;
    uint32_t  tx_pkt;
    uint32_t  tx_drops;
} cc_ofstats_t;

/* node in ofrw_htbl */
typdef struct cc_ofrw_info_ {
    cc_ofrw_key_t        key;
    cc_ofrw_state_e      state;
    adpoll_thread_mgr_t  *thr_mgr_p;
    

} cc_ofrw_info_t;

typedef struct cc_ofdev_key_ {
    ipaddr_v4v6_t  controller_ip_addr;
    ipaddr_v4v6_t  switch_ip_addr;
    L4_type_e      layer4_proto;    
} cc_ofdev_key_t;

typedef enum cc_ofver_ {
    CC_OFVER_1_0   = 0,
    CC_OFVER_1_3,
    CC_OFVER_1_3_1
} cc_ofver_e;

/* node in ofdev_htbl */
typedef struct cc_ofdev_info_ {
    cc_ofdev_key_t key;

    uint16_t       controller_L4_port;

    cc_ofver_e     of_max_ver;
    
    uint32_t       count_rwsockets;
    GList          *ofrw_socket_list; //list of rw sockets
    //LOCK for htbl

    int            main_sockfd;
} cc_ofdev_info_t;


/* mapping of channel key (dp-id/aux-id) to rw_sockfd) */
typedef struct cc_ofchannel_key_ {
    uint64_t  dp_id;
    uint8_t   aux_id;
}cc_ofchannel_key_t;

typedef struct cc_ofchann_info_ {
    cc_ofchannel_key_t    key;
    int                   rw_sockfd;
    int                   count_retries; /* CLIENT: reconnection attempts */
    cc_ofstats_t          stats;    
} cc_ofchannel_info_t;

typedef struct cc_of_global_ {
    /* driver type could be controller or switch */
    of_drv_type_e     ofdrv_type;

    /* layer4 device type could be client or server */
    of_dev_type_e     ofdev_type;

    /* node: cc_ofdev_info_t */
    GHashTable       *ofdev_htbl;
    uint32_t         count_devs;
    GMutex           ofdev_htbl_lock;

    /* node:  cc_ofchannel_info_t */
    GHashTable       *ofchannel_htbl;
    uint32_t         count_ofchannels;
    GMutex	     ofchannel_htbl_lock;

    /*node: cc_ofrw_info_t */
    GHashTable       *ofrw_htbl;
    uint32_t         count_ofrw;
    GMutex           ofrw_htbl_lock;
    
    net_svcs_t       NET_SVCS[MAX_L4_TYPE];

    adpoll_thread_mgr_t  *oflisten_pollthr_p; /* NULL for client */
    
    GList            *ofrw_pollthr_list;
    uint32_t         count_pollthr;
    
} cc_of_global_t;

extern cc_of_global_t cc_of_global;

#endif //CC_OF_GLOBAL_H
