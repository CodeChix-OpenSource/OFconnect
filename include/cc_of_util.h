/*
****************************************************************
**      CodeChix OFconnect - OpenFlow Channel Management Library
**      Copyright CodeChix 2013-2014
**      codechix.org - May the code be with you...
****************************************************************
**
** License:             GPL v2
** Version:             1.0
** Project/Library:     OFconnect, libccof.so
** GLIB License:        GNU LGPL
** Description:    	Common utilities definitions
** Assumptions:         N/A
** Testing:             N/A
**
** Main Contact:        deepa.dhurka@gmail.com
** Alt. Contact:        organizers@codechix.org
****************************************************************
*/

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

#define MAX_OFRW_HTBL_SIZE 10007
#define MAX_OFCHANN_HTBL_SIZE 10007
#define MAX_OFDEV_HTBL_SIZE 101

typedef enum htbl_update_ops_ {
    ADD,
    DEL,
    UPD
} htbl_update_ops_e;

typedef enum htbl_type_ {
    OFDEV,
    OFRW,
    OFCHANN
} htbl_type_e;

guint cc_ofdev_hash_func(gconstpointer key);

guint cc_ofchann_hash_func(gconstpointer key);

guint cc_ofrw_hash_func(gconstpointer key);

void cc_of_destroy_generic(gpointer data);

void cc_ofdev_htbl_destroy_val(gpointer data);

gboolean cc_ofdev_htbl_equal_func(gconstpointer a, 
                                  gconstpointer b);

gboolean cc_ofchannel_htbl_equal_func(gconstpointer a, 
                                      gconstpointer b);

gboolean cc_ofrw_htbl_equal_func(gconstpointer a, 
                                 gconstpointer b);

cc_of_ret
update_global_htbl(htbl_type_e htbl_type,
                   htbl_update_ops_e htbl_op,
                   gpointer htbl_key,
                   gpointer htbl_data,
                   gboolean *new_entry);

cc_of_ret
update_global_htbl_lockfree(htbl_type_e htbl_type,
                            htbl_update_ops_e htbl_op,
                            gpointer htbl_key,
                            gpointer htbl_data,
                            gboolean *new_entry);

void
print_ofdev_htbl(void);
void
print_ofrw_htbl(void);
void
print_ofchann_htbl(void);



void
print_ofrw_htbl(void);

void
print_ofchann_htbl(void);



cc_of_ret
del_ofrw_rwsocket(int del_fd);

cc_of_ret
add_upd_ofrw_ofdev_rwsocket(int add_fd);

cc_of_ret
find_thrmgr_rwsocket(int sockfd,  
                     adpoll_thread_mgr_t **tmgr);

cc_of_ret
find_thrmgr_rwsocket_lockfree(int sockfd,  
                     adpoll_thread_mgr_t **tmgr);

cc_of_ret
add_upd_ofchann_rwsocket(cc_ofchannel_key_t key,
                         int rwsock);

cc_of_ret
del_ofchann_rwsocket(int rwsock);

cc_of_ret
find_ofchann_key_rwsocket(int sockfd, 
                          cc_ofchannel_key_t **fd_chann_key);

cc_of_ret
add_ofdev_rwsocket(cc_ofdev_key_t key, int rwsock);

cc_of_ret
del_ofdev_rwsocket(cc_ofdev_key_t key, int rwsock);

cc_of_ret
atomic_add_upd_htbls_with_rwsocket(int sockfd, struct sockaddr_in *client_addr, 
                                   adpoll_thread_mgr_t  *thr_mgr,
                                   cc_ofdev_key_t key,
                                   L4_type_e layer4_proto, 
                                   cc_ofchannel_key_t ofchann_key);

/*-----------------------------------------------------------------------*/
/* POLLTHR utilities                                                     */
/* Utilities to manage the rw poll thr pool                              */
/* Sorted list of rw pollthreads                                         */
/* Sorted based on num available socket fds - largest availability first */
/*-----------------------------------------------------------------------*/

cc_of_ret
cc_create_rw_pollthr(adpoll_thread_mgr_t **tmgr);

guint
cc_get_count_rw_pollthr(void);


cc_of_ret
cc_find_or_create_rw_pollthr(adpoll_thread_mgr_t **tmgr);


cc_of_ret
cc_del_sockfd_rw_pollthr(adpoll_thread_mgr_t *tmgr, 
                         adpoll_thr_msg_t *msg);


cc_of_ret
cc_add_sockfd_rw_pollthr(adpoll_thr_msg_t *msg, 
                         cc_ofdev_key_t key, 
                         L4_type_e layer4_proto,
                         cc_ofchannel_key_t ofchann_key);

#endif //CC_OF_UTIL_H
