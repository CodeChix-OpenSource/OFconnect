/*-----------------------------------------------------------------------------*/
/* Copyright: CodeChix Bay Area Chapter 2013                                   */
/*-----------------------------------------------------------------------------*/
#ifndef CC_OF_UTIL_H
#define CC_OF_UTIL_H

#include "cc_of_global.h"
#include "cc_net_conn.h"
#include "cc_pollthr_mgr.h"
#include "cc_log.h"

#ifndef UNUSED
#define UNUSED __attribute__ ((__unused__))
#endif

/* HTBL utilities */
typedef enum htbl_update_ops_ {
    ADD,
    DEL
} htbl_update_ops_e;

typedef enum htbl_type_ {
    OFDEV,
    OFRW,
    OFCHANN
} htbl_type_e;

cc_of_ret
update_global_htbl(htbl_type_e htbl_type,
                   htbl_update_ops_e htbl_op,
                   gpointer htbl_key,
                   gpointer htbl_data,
                   gboolean *new_entry);

cc_of_ret
del_ofrw_rwsocket(int del_fd);

cc_of_ret
add_upd_ofrw_ofdev_rwsocket(int add_fd);

cc_of_ret
add_upd_ofchann_rwsocket(cc_ofchannel_key_t key,
                         int rwsock);

cc_of_ret
del_ofchann_rwsocket(int rwsock);

cc_of_ret
add_ofdev_rwsocket(cc_ofdev_key_t key, int rwsock);

cc_of_ret
del_ofdev_rwsocket(cc_ofdev_key_t key, int rwsock);

cc_of_ret
atomic_add_upd_ofrw_ofdev_rwsocket(int sockfd, 
				   adpoll_thread_mgr_t  *thr_mgr,
				   cc_ofdev_key_t key);

/* POLLTHR utilities */
cc_of_ret
cc_create_rw_pollthr(adpoll_thread_mgr_t **tmgr,
                     int32_t max_sockets,
                     uint32_t max_pipes);


guint
cc_get_count_rw_pollthr(void);


cc_of_ret
cc_find_or_create_rw_pollthr(adpoll_thread_mgr_t **tmgr,
                            uint32_t max_sockets,
                            uint32_t max_pipes);


cc_of_ret
cc_del_sockfd_rw_pollthr(adpoll_thread_mgr_t *tmgr, adpoll_thr_msg_t *msg);


cc_of_ret
cc_add_sockfd_rw_pollthr(adpoll_thr_msg_t *msg, cc_ofdev_key_t key);


#endif //CC_OF_UTIL_H
