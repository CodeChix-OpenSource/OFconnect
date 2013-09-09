/*-----------------------------------------------------------------------------*/
/* Copyright: CodeChix Bay Area Chapter 2013                                   */
/*-----------------------------------------------------------------------------*/
#ifndef CC_OF_GLOBAL_H
#define CC_OF_GLOBAL_H

#include <glib.h>
#include "cc_net_conn.h"
#include "cc_of_lib.h"
#include "cc_pollthr_mgr.h"

#define SIZE_RW_THREAD_BUCKET        20

#define MAX_PER_THREAD_RWSOCKETS     200

#define MAX_PER_THREAD_PIPES          0 /* no additional pipes */

typedef struct cc_of_global_ {
    /* layer4 device type could be switch or controller */
    of_dev_type_e     ofdev_type;

    /* node: cc_ofdev_info_t */
    GHashTable       *ofdev_htbl;
    GMutex           ofdev_htbl_lock;

    /* node:  cc_ofchannel_info_t */
    GHashTable       *ofchannel_htbl;
    GMutex	         ofchannel_htbl_lock;

    /*node: cc_ofrw_info_t */
    GHashTable       *ofrw_htbl;
    GMutex           ofrw_htbl_lock;
    
    net_svcs_t       NET_SVCS[MAX_L4_TYPE];

    adpoll_thread_mgr_t  *oflisten_pollthr_p; /* NULL for client */
    
    GList            *ofrw_pollthr_list; /* adpoll_thread_mgr elems */

    /* debugs and log file */
    FILE             *oflog_fd;
    char             *oflog_file;
    gboolean         ofdebug_enable; /* enable debugging */
    gboolean         oflog_enable; /* enable logging to file */
    GMutex           oflog_lock;
    gboolean         ofut_enable; /* extra debugging to support ut */
} cc_of_global_t;

extern cc_of_global_t cc_of_global;

#endif //CC_OF_GLOBAL_H
