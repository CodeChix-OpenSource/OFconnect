/*-----------------------------------------------------------------------------*/
/* Copyright: CodeChix Bay Area Chapter 2013                                   */
/*-----------------------------------------------------------------------------*/
#include "cc_of_util.h"

/*-----------------------------------------------------------------------*/
/* Utilities to manage the rw poll thr pool                              */
/* Sorted list of rw pollthreads                                         */
/* Sorted based on num available socket fds - largest availability first */
/*-----------------------------------------------------------------------*/
cc_drv_ret
cc_create_rw_pollthr(adpoll_thread_mgr_t *tmgr,
                     uint32_t max_sockets,
                     uint32_t max_pipes)
{
    char tname[MAX_NAME_LEN];

    g_sprintf(tname,"rwthr_%3d", cc_of_global.count_pollthr + 1);
    
    tmgr = adp_thr_mgr_new(tname, max_sockets, max_pipes);

    if (tmgr == NULL) {
        CC_LOG_ERROR("%s(%d): failed to create new poll thread for rw");
        return(CC_ERR);
    }

    /* update the global GList */
    cc_of_global.ofrw_pollthr_list = g_list_prepend(cc_of_global.ofrw_pollthr_list,
                                                    (gpointer)tmgr);
    cc_of_global.count_pollthr++;

    CC_LOG_DEBUG("%s(%d): created new rw pollthr %s. total global count: %d",
                 __FUNCTION__, __LINE__, tname, cc_of_global.count_pollthr);

    return(CC_OK);
}

uint32_t
cc_get_count_rw_pollthr(void)
{
    return(cc_of_global.count_pollthr);
}

cc_drv_ret
cc_find_or_create_rw_pollthr(adpoll_thread_mgr_t *tmgr,
                            uint32_t max_sockets,
                            uint32_t max_pipes)
{
    GList *elem, *next_elem;
    if (cc_of_global.count_pollthr == 0) {
        CC_LOG_DEBUG("%s(%d): no existing poll thr - create new",
                     __FUNCTION__, __LINE__);
        return(cc_create_rw_pollthr(tmgr, max_sockets, max_pipes));
    }    

    elem = g_list_first(cc_of_global.ofrw_pollthr_list);
    g_assert(elem != NULL);

    if (adp_thr_mgr_get_num_avail_sockfd(
            (adpoll_thread_mgr_t *)(elem->data)) == 0) {
        CC_OF_DEBUG("%s(%d) - socket capacity exhausted. create new poll thr",
                    __FUNCTION__, __LINE__);
        return(cc_create_rw_pollthr(tmgr, max_sockets, max_pipes));
    }    
        
    while (((next_elem = g_list_next(elem)) != NULL) &&
           (adp_thr_mgr_get_num_avail_sockfd(
               (adpoll_thread_mgr_t *)(next_elem->data)) != 0)) {
        elem = next_elem;
    }
    
    tmgr = (adpoll_thread_mgr_t *)(elem);
    
    return(CC_OK);
}

gint
cc_pollthr_list_compare_func(adpoll_thread_mgr_t *tmgr1,
                             adpoll_thread_mgr_t *tmgr2)
{
    uint32_t tmgr1_avail, tmgr2_avail;

    tmgr1_avail = adp_thr_mgr_get_num_avail_sockfd(tmgr1);
    tmgr2_avail = adp_thr_mgr_get_num_avail_sockfd(tmgr2);
    
    if (tmgr1_avail > tmgr2_avail) {
        return -1;
    } else if (tmgr1_avail < tmgr2_avail) {
        return 1;
    }
    return 0;
}

cc_drv_ret
add_ofdev_rwsocket();


cc_drv_ret
del_ofdev_rwsocket();

cc_drv_ret
add_ofrw_rwsocket();

static gboolean
ofrw_htbl_remove_callback(gpointer key, gpointer value, gpointer user_data)
{
    cc_ofrw_key_t *ofrw_key;
    ofrw_key = (cc_ofrw_key_t *)key;

    if (ofrw_key->rw_sockfd == *(int *)user_data) {
        return TRUE;
    }
    return FALSE;
}

guint
del_ofrw_rwsocket(int del_fd)
{
    return(g_hash_table_foreach_remove(cc_of_global.ofrw_htbl,
                                       (GHRFunc) del_ofrw_rwsocket,
                                       (gpointer) &del_fd));
}                                        

cc_drv_ret
add_ofchann_rwsocket(cc_ofchannel_key_t key,
                     int rwsock)
{
    

}

cc_drv_ret
del_ofchann_rwsocket(int rwsock);

cc_drv_net
del_ofchann(cc_ofchannel_key_t key);

cc_drv_ret
cc_del_sockfd_rw_pollthr(adpoll_thread_mgr_t *tmgr, int fd)
{
    adpoll_thr_msg_t del_fd_msg;
    
    del_fd_msg.fd_type = SOCKET;
    del_fd_msg.fd_action = DELETE;
    del_fd_msg.fd = fd;
    
    adp_thr_mgr_add_del_fd(tmgr, &del_fd_msg);

    if (cc_of_global.count_pollthr == 1) {
        /* no sorting required with only 1 thread */
        CC_LOG_DEBUG("%s(%d): only one poll thread. skip sorting",
                     __FUNCTION__, __LINE__);
        return(CC_OK);
    }

    cc_of_global.ofrw_pollthr_list = g_list_remove(cc_of_global.ofrw_pollthr_list,
                                                   tmgr);
    cc_of_global.ofrw_pollthr_list = g_list_insert_sorted(
        cc_of_global.ofrw_pollthr_list,
        tmgr,
        (GCompareFunc)cc_pollthr_list_compare_func);

    /* TODO: clean out this fd from global structures */
//    ofrw_socket_list in device
//    cc_of_global.ofrw_htbl - cc_ofrw_info_t
    //delete from ofchannel_htbl - cc_ofchannel_info_t

    return (CC_OK);

}

cc_drv_ret
cc_add_sockfd_rw_pollthr(adpoll_thr_msg_t add_fd_msg)
{
    /* find or create a poll thread */
    /* add the fd to it */
    /* add to global structures */
}
