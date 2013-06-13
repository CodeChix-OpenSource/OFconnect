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

typedef enum of_dev_type_ {
    SWITCH = 0,
    CONTROLLER,
    MAX_OF_DEV_TYPE
} of_dev_type_e;

typedef enum cc_ofchannel_state_ {
    CC_OF_CHANNEL_DOWN = 0,
    CC_OF_CHANNEL_UP
} cc_ofchannel_state_e;

typdef struct cc_ofchannel_key_ {
    int       rw_sockfd;    
} cc_of_channel_key_t;

/* node in ofrw_htbl */
typdef struct cc_ofrw_info_ {
    cc_ofchannel_key_t   key;
    cc_ofchannel_state_e state;
    GThread              *thread_p;

    //RD pipe to thread
    //WR pipe to thread
    //synchronization vars

    /* stats */
} cc_ofrw_info_t;

typedef struct cc_ofdev_key_ {
    ipaddr_v4v6_t  controller_ip_addr;
    ipaddr_v4v6_t  switch_ip_addr; /*UNUSED for dev type CONTROLLER */
    L4_type_e      layer4_proto;    
} cc_ofdev_key_t

/* node in ofdev_htbl */
typedef struct cc_ofdev_info_ {
    cc_ofdev_key_t key;
    uint16_t       controller_L4_port;

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
} cc_ofchannel_info_t;


typedef struct cc_of_global_ {
    /* device type could be controller or switch */
    of_drv_type_e     ofdrv_type;

    /* controller: spawns multiple IP address endpoints
     *             for the same controller
     *             (multiple instances of same controller)
     * switch:
     */
    GHashTable       *ofdev_htbl;
    uint32_t         count_devs;
    //LOCK for htbl

    GHashTable       *ofchannel_htbl;
    uint32_t         count_ofchannels;
    //LOCK for htbl

    GHashTable       *ofrw_htbl;
    uint32_t         count_ofrw;
    
    net_svcs_t       NET_SVCS[MAX_OF_DRV_TYPE][MAX_L4_TYPE];
    
} cc_of_global_t;

#endif //CC_OF_GLOBAL_H
