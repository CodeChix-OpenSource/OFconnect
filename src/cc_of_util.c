/*-----------------------------------------------------------------------------*/
/* Copyright: CodeChix Bay Area Chapter 2013                                   */
/*-----------------------------------------------------------------------------*/
#include "cc_of_util.h"

/* Utilities to manage the rw poll thr pool */
/* Sorted list of rw pollthreads */
/* Sorted based on num available socket fds - largest availability first */
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
    g_list_prepend(cc_of_global.ofrw_pollthr_list,
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
cc_get_or_create_rw_pollthr(adpoll_thread_mgr_t *tmgr,
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

    while (((next_elem = g_list_next(elem)) != NULL) &&
           (adp_thr_mgr_get_num_avail_sockfd(
               (adpoll_thread_mgr_t *)(next_elem->data)) != 0)) {
        elem = next_elem;
    }
    
    tmgr = (adpoll_thread_mgr_t *)(elem);
    
    return(CC_OK);
}

cc_drv_ret
cc_del_sockfd_rw_pollthr(
