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

typdef struct cc_ofchannel_key_ {
    uint64_t  dp_id;
    uint8_t   aux_id;
} cc_of_channel_key_t;

typedef enum cc_ofchannel_state_ {
    CC_OF_CHANNEL_DOWN = 0,
    CC_OF_CHANNEL_UP
} cc_ofchannel_state_e;


/* node in ofchannel_htbl */
typdef struct cc_ofchannel_info_ {
    cc_ofchannel_key_t   key;
    cc_ofchannel_state_e state;
    GThread              *thread_p;
    L4_type_e            layer4_proto;
    int                  rw_sockfd;
    //RD pipe to thread
    //WR pipe to thread
    //synchronization vars

    /* stats */

} cc_ofchannel_info_t;


typedef struct cc_ofdev_key_ {
    ipaddr_v4v6_t  controller_ip_addr;
    ipaddr_v4v6_t  switch_ip_addr; /*UNUSED for dev type CONTROLLER */
} cc_ofdev_key_t

/* node in ofdev_htbl */
typedef struct cc_ofdev_info_ {
    cc_ofdev_key_t key;
    uint16_t       controller_L4_ports[MAX_L4_TYPE];
    uint16_t       switch_L4_port[MAX_L4_TYPE];

    uint32_t       count_channels;
    GHashTable     *ofchannel_htbl;

    int            main_sockfd;
} cc_ofdev_info_t;


typedef struct cc_of_global_ {
    /* device type could be controller or switch */
    of_drv_type_e     ofdrv_type;

    /* controller: spawns multiple IP address endpoints
     *             for the same controller
     *             (multiple instances of same controller)
     * switch:
     */
    GHashTable       *ofdev_htbl;

    net_svcs_t       NET_SVCS[MAX_OF_DRV_TYPE][MAX_L4_TYPE];
    
} cc_of_global_t;

#endif //CC_OF_GLOBAL_H
