/*-----------------------------------------------------------------------------*/
/* Copyright: CodeChix Bay Area Chapter 2013                                   */
/*-----------------------------------------------------------------------------*/
#include "cc_of_util.h"
/*-----------------------------------------------------------------------*/
/* Utilities to manage the global hash tables                            */
/*-----------------------------------------------------------------------*/

cc_of_ret
update_global_htbl(htbl_type_e htbl_type,
                   htbl_update_ops_e htbl_op,
                   gpointer htbl_key,
                   gpointer htbl_data,
                   gboolean *new_entry)
{
    GHashTable *cc_htbl;
    GMutex *cc_htbl_lock;

    *new_entry = FALSE;
    guint old_count;
    
    switch(htbl_type) {
      case OFDEV:
        cc_htbl = cc_of_global.ofdev_htbl;
        cc_htbl_lock = &cc_of_global.ofdev_htbl_lock;
        break;
      case OFRW:
        cc_htbl = cc_of_global.ofrw_htbl;
        cc_htbl_lock = &cc_of_global.ofrw_htbl_lock;
        break;
      case OFCHANN:
        cc_htbl = cc_of_global.ofchannel_htbl;
        cc_htbl_lock = &cc_of_global.ofchannel_htbl_lock;
        break;
      default:
        CC_LOG_ERROR("%s(%d): invalid htbl type %d",
                     __FUNCTION__, __LINE__, htbl_type);
    }

    if (htbl_op == ADD) {
        g_mutex_lock(cc_htbl_lock);
        old_count = g_hash_table_size(cc_htbl);
        g_hash_table_insert(cc_htbl, htbl_key, htbl_data);
        if (g_hash_table_size(cc_htbl) > old_count) {
            *new_entry = TRUE;
        }
        g_mutex_unlock(cc_htbl_lock);

    } else if (htbl_op == DEL) {
        g_mutex_lock(cc_htbl_lock);
        if (g_hash_table_remove(cc_htbl, htbl_key) == FALSE) {
            return CC_OF_EHTBL;
        }
        g_mutex_unlock(cc_htbl_lock);
    }
    return CC_OF_OK;
}

cc_of_ret
del_ofrw_rwsocket(int del_fd)
{
    cc_ofrw_key_t ofrw_key;
    ofrw_key.rw_sockfd = del_fd;
    gboolean new_entry;

    return (update_global_htbl(OFRW, DEL, (gpointer)&ofrw_key,
                               NULL, &new_entry));
}

cc_of_ret
add_upd_ofrw_rwsocket(int add_fd, adpoll_thread_mgr_t  *thr_mgr_p,
                      L4_type_e layer4_proto,
                      cc_ofdev_key_t key)
{
    cc_ofrw_key_t *ofrw_key;
    cc_ofrw_info_t *ofrw_info;
    gboolean new_entry;
    cc_of_ret rc;
    
    ofrw_key = (cc_ofrw_key_t *)g_malloc(sizeof(cc_ofrw_key_t));
    ofrw_info = (cc_ofrw_info_t *)g_malloc(sizeof(cc_ofrw_info_t));

    ofrw_key->rw_sockfd = add_fd;
    ofrw_info->state = CC_OF_RW_DOWN;
    ofrw_info->thr_mgr_p = thr_mgr_p;
    memcpy(&(ofrw_info->dev_key), &key, sizeof(cc_ofdev_key_t));
    ofrw_info->layer4_proto = layer4_proto;

    rc = update_global_htbl(OFRW, ADD,
                            ofrw_key, ofrw_info,
                            &new_entry);
    if (!new_entry) {
        g_free(ofrw_key);
    }
    return rc;
}

cc_of_ret
find_thrmgr_rwsocket(int sockfd, 
                     adpoll_thread_mgr_t **tmgr) {
    cc_of_ret status = CC_OF_OK;
    cc_ofrw_key_t rwkey;
    cc_ofrw_info_t *rwinfo = NULL;
    
    rwkey.rw_sockfd = sockfd;
    rwinfo = g_hash_table_lookup(cc_of_global.ofrw_htbl, &rwkey);
    if (rwinfo == NULL) {
        CC_LOG_ERROR("%s(%d): could not find rwsock %d in ofrw_htbl",
                     __FUNCTION__, __LINE__, rwkey.rw_sockfd);
        return CC_OF_EHTBL;
    }
    
    *tmgr = rwinfo->thr_mgr_p;
    return status;
}

cc_of_ret
add_upd_ofchann_rwsocket(cc_ofchannel_key_t key,
                         int rwsock)
{
    cc_ofchannel_key_t *ofchannel_key;
    cc_ofchannel_info_t *ofchannel_info;
    gboolean new_entry;
    cc_of_ret rc;

    ofchannel_key = (cc_ofchannel_key_t *)g_malloc(sizeof
                                                   (cc_ofchannel_key_t));
    ofchannel_info = (cc_ofchannel_info_t *)g_malloc(sizeof
                                                     (cc_ofchannel_info_t));

    memcpy(ofchannel_key, &key, sizeof(cc_ofchannel_key_t));
    ofchannel_info->rw_sockfd = rwsock;
    ofchannel_info->count_retries = 0;
//    ofchannel_info->stats = {0}; /* was giving compilation err altho looks correct! */
    ofchannel_info->stats.rx_pkt = 0;
    ofchannel_info->stats.tx_pkt = 0;
    ofchannel_info->stats.tx_drops = 0;

    rc = update_global_htbl(OFCHANN, ADD,
                            ofchannel_key, ofchannel_info,
                            &new_entry);
    if (!new_entry) {
        g_free(ofchannel_key);
    }
    return rc;
}

gboolean
ofchannel_entry_match(cc_ofchannel_key_t *key UNUSED,
                      cc_ofchannel_info_t *info,
                      int *rwsock)
{
    if (info->rw_sockfd == *rwsock) {
        return TRUE;
    }
    return FALSE;
}

cc_of_ret
del_ofchann_rwsocket(int rwsock)
{
    g_hash_table_foreach_remove(cc_of_global.ofchannel_htbl,
                                (GHRFunc)ofchannel_entry_match,
                                &rwsock);
    return CC_OF_OK;
}

cc_of_ret
find_ofchann_key_rwsocket(int sockfd, cc_ofchannel_key_t **fd_chann_key) {

    cc_of_ret status = CC_OF_OK;
    cc_ofchannel_key_t *channel_key_tmp = NULL;
    GHashTableIter ofchannel_iter;
    cc_ofchannel_key_t *channel_key = NULL;
    cc_ofchannel_info_t *channel_info = NULL;

    /*   
     * Do a reverse lookup to get the channel_key
     * corresponding to this sockfd
     */

    g_hash_table_iter_init(&ofchannel_iter, cc_of_global.ofchannel_htbl);
    if (g_hash_table_iter_next(&ofchannel_iter, (gpointer *)&channel_key, (gpointer *)&channel_info)) {
        if (channel_info->rw_sockfd == sockfd) {
            channel_key_tmp = channel_key;
        }
    }

    if (channel_key_tmp == NULL) {
        CC_LOG_ERROR("%s(%d): could not find channel_key for rwsock %d"
                     , __FUNCTION__, __LINE__, sockfd);
        return CC_OF_EHTBL;
    }

    *fd_chann_key = channel_key_tmp;
    return status;
}


cc_of_ret add_ofdev_rwsocket(cc_ofdev_key_t key, int rwsock)
{
    cc_ofdev_info_t *ofdev = NULL;
    int *list_elem;
    gboolean new_entry;

    ofdev = g_hash_table_lookup(cc_of_global.ofdev_htbl,
                                &key);
    if (!ofdev) {
        return CC_OF_EINVAL;
    }

    list_elem = (int *)malloc(sizeof(int));
    *list_elem = rwsock;
    
    g_mutex_lock(&ofdev->ofrw_socket_list_lock);
    ofdev->ofrw_socket_list = g_list_append(ofdev->ofrw_socket_list,
                                                  list_elem);
    g_mutex_unlock(&ofdev->ofrw_socket_list_lock);

    return(update_global_htbl(OFDEV, ADD, &key, &ofdev, &new_entry));
}

cc_of_ret
del_ofdev_rwsocket(cc_ofdev_key_t key, int rwsock)
{
    cc_ofdev_info_t *ofdev = NULL;
    GList *tmp_list = NULL;
    int *tmp_rwsock = NULL;
    gboolean new_entry;

    ofdev = g_hash_table_lookup(cc_of_global.ofdev_htbl,
                                &key);
    if (!ofdev) {
        return CC_OF_EINVAL;
    }

    g_mutex_lock(&ofdev->ofrw_socket_list_lock);
    tmp_list = g_list_find(ofdev->ofrw_socket_list, &rwsock);
    if (tmp_list == NULL) {
        CC_LOG_ERROR("%s(%d): could not find rwsock %d "
                     "in ofrw_socket_list",
                     __FUNCTION__, __LINE__, rwsock);
        g_mutex_unlock(&ofdev->ofrw_socket_list_lock);
        return CC_OF_EMISC;
    }

    tmp_rwsock = (int *)tmp_list->data;
    ofdev->ofrw_socket_list = g_list_delete_link(ofdev->ofrw_socket_list,
                                                 tmp_list);
    free(tmp_rwsock);
    g_mutex_unlock(&ofdev->ofrw_socket_list_lock);
    
    return(update_global_htbl(OFDEV, ADD, &key, &ofdev, &new_entry));
}

cc_of_ret
atomic_add_upd_htbls_with_rwsocket(int sockfd, adpoll_thread_mgr_t  *thr_mgr, 
                                   cc_ofdev_key_t key, L4_type_e layer4_proto,
                                   cc_ofchannel_key_t ofchann_key)
{
    cc_of_ret status = CC_OF_OK;

    if((status = add_upd_ofrw_rwsocket(sockfd, thr_mgr, layer4_proto, key)) < 0) {
        CC_LOG_ERROR("%s(%d): %s", __FUNCTION__, __LINE__, cc_of_strerror(status));
        return status;
    } 

	if ((status = add_ofdev_rwsocket(key, sockfd)) < 0) {
	     CC_LOG_ERROR("%s(%d): %s", __FUNCTION__, __LINE__, cc_of_strerror(status));
	     del_ofrw_rwsocket(sockfd);
	     return status;
	} 
    
    /* update ofchannel htbl */

    /* If controller, use sockfd as dummy dp_id/aux_id until we recv 
     * first mesg on this fd and then update the actual aux_id/dp_id 
     * in the ofchann_key.
     *
     * If switch, then use the dp_id/aux_id sent by the switch itself
     */
    if (cc_of_global.ofdev_type == CONTROLLER) {
        cc_ofchannel_key_t controller_chann_key;

        controller_chann_key.dp_id = (uint64_t)sockfd;
        controller_chann_key.aux_id = (uint8_t)sockfd;
        if ((status = add_upd_ofchann_rwsocket(controller_chann_key, sockfd)) < 0) {
            CC_LOG_ERROR("%s(%d): %s", __FUNCTION__, __LINE__, cc_of_strerror(status));
            del_ofdev_rwsocket(key, sockfd);
            del_ofrw_rwsocket(sockfd);
            return status;
        }
    } else if (cc_of_global.ofdev_type == SWITCH) {
        if ((status = add_upd_ofchann_rwsocket(ofchann_key, sockfd)) < 0) {
            CC_LOG_ERROR("%s(%d): %s", __FUNCTION__, __LINE__, cc_of_strerror(status));
            del_ofdev_rwsocket(key, sockfd);
            del_ofrw_rwsocket(sockfd);
            return status;
        }
    }

	CC_LOG_DEBUG("%s(%d): Atomically added sockfd to ofrw & ofdev"
                 "htbls", __FUNCTION__, __LINE__);
    
    return status;
}



/*-----------------------------------------------------------------------*/
/* Utilities to manage the rw poll thr pool                              */
/* Sorted list of rw pollthreads                                         */
/* Sorted based on num available socket fds - largest availability first */
/*-----------------------------------------------------------------------*/
guint
cc_get_count_rw_pollthr(void)
{
    return(g_list_length(cc_of_global.ofrw_pollthr_list));
}

cc_of_ret
cc_create_rw_pollthr(adpoll_thread_mgr_t **tmgr)
{
    adpoll_thread_mgr_t *tmgr_new = NULL;
    char tname[MAX_NAME_LEN];
    int max_sockets = MAX_PER_THREAD_RWSOCKETS;
    int max_pipes = MAX_PER_THREAD_PIPES;

    g_sprintf(tname,"rwthr_%3d", cc_get_count_rw_pollthr() + 1);
    
    tmgr_new = adp_thr_mgr_new(tname, max_sockets, max_pipes);

    if (tmgr_new == NULL) {
        CC_LOG_ERROR("%s(%d): failed to create new poll thread for rw",
                     __FUNCTION__, __LINE__);
        return CC_OF_EMISC;
    }

    /* update the global GList */
    cc_of_global.ofrw_pollthr_list =
        g_list_prepend(cc_of_global.ofrw_pollthr_list, (gpointer)tmgr_new);
    
    CC_LOG_DEBUG("%s(%d): created new rw pollthr %s "
                 "total pollthr %d",
                 __FUNCTION__, __LINE__, tname,
                 cc_get_count_rw_pollthr());

    if (tmgr != NULL)
        *tmgr = tmgr_new;
    return(CC_OF_OK);
}


cc_of_ret
cc_find_or_create_rw_pollthr(adpoll_thread_mgr_t **tmgr)
{
    GList *elem, *next_elem;

    if (cc_get_count_rw_pollthr() == 0) {
        CC_LOG_DEBUG("%s(%d): no existing poll thr - create new",
                     __FUNCTION__, __LINE__);
        return(cc_create_rw_pollthr(tmgr));
    }    

    elem = g_list_first(cc_of_global.ofrw_pollthr_list);
    g_assert(elem != NULL);

    if (adp_thr_mgr_get_num_avail_sockfd(
            (adpoll_thread_mgr_t *)(elem->data)) == 0) {
        CC_LOG_DEBUG("%s(%d) - socket capacity exhausted. create new poll thr",
    /* TODO: Finish this impl */
                    __FUNCTION__, __LINE__);
        return(cc_create_rw_pollthr(tmgr));
    }    
        
    while (((next_elem = g_list_next(elem)) != NULL) &&
           (adp_thr_mgr_get_num_avail_sockfd(
               (adpoll_thread_mgr_t *)(next_elem->data)) != 0)) {
        elem = next_elem;
    }
    
    *tmgr = (adpoll_thread_mgr_t *)(elem);
    
    return(CC_OF_OK);
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

cc_of_ret
cc_del_sockfd_rw_pollthr(adpoll_thread_mgr_t *tmgr, adpoll_thr_msg_t *thr_msg)
{
    cc_ofrw_key_t rwkey;
    cc_ofrw_info_t *rwinfo_p = NULL;
    GList *tmp_list = NULL;
    adpoll_thread_mgr_t  *tmp_tmgr = NULL;
    
    CC_LOG_DEBUG("%s(%d) Thread: %s, fd: %d, type: %s", __FUNCTION__,
                 __LINE__, tmgr->tname, thr_msg->fd,
                 (thr_msg->fd_type == PIPE)? "pipe":"socket");

    if ((tmgr == NULL) || (thr_msg == NULL)) {
        CC_LOG_ERROR("%s(%d): invalid parameters",
                     __FUNCTION__, __LINE__);
        return CC_OF_EINVAL;
    }
    if (thr_msg->fd_action != DELETE_FD) {
        CC_LOG_ERROR("%s(%d): incorrect action request",
                     __FUNCTION__, __LINE__);
        return CC_OF_EINVAL;
    }
    
    adp_thr_mgr_add_del_fd(tmgr, thr_msg);
    
    if (cc_get_count_rw_pollthr() == 1) {
        // no sorting required with only 1 thread 
        CC_LOG_DEBUG("%s(%d): only one poll thread. skip sorting",
                     __FUNCTION__, __LINE__);
        return(CC_OF_OK);
    }

    tmp_list = g_list_find(cc_of_global.ofrw_pollthr_list, tmgr);
    if (tmp_list == NULL) {
        CC_LOG_ERROR("%s(%d): could not find thread manager %s "
                     "in ofrw_pollthr_list",
                     __FUNCTION__, __LINE__, tmgr->tname);
        return(CC_OF_EMISC);
    }
    tmp_tmgr = (adpoll_thread_mgr_t *)tmp_list->data;
    cc_of_global.ofrw_pollthr_list =
        g_list_delete_link(cc_of_global.ofrw_pollthr_list, tmp_list);
    free(tmp_tmgr);
    
    cc_of_global.ofrw_pollthr_list = g_list_insert_sorted(
        cc_of_global.ofrw_pollthr_list,
        tmgr,
        (GCompareFunc)cc_pollthr_list_compare_func);

    rwkey.rw_sockfd = thr_msg->fd;
    rwinfo_p = g_hash_table_lookup(cc_of_global.ofrw_htbl, &rwkey);

    if (rwinfo_p) {
        //ofrw_socket_list in device
        del_ofdev_rwsocket(rwinfo_p->dev_key, thr_msg->fd);

    }
    //cc_of_global.ofrw_htbl - cc_ofrw_info_t
    del_ofrw_rwsocket(thr_msg->fd);
    
    //delete from ofchannel_htbl - cc_ofchannel_info_t
    del_ofchann_rwsocket(thr_msg->fd);

    return (CC_OF_OK);

}

cc_of_ret
cc_add_sockfd_rw_pollthr(adpoll_thr_msg_t *thr_msg, cc_ofdev_key_t key,
                         L4_type_e layer4_proto, cc_ofchannel_key_t ofchann_key)
{
    cc_of_ret status = CC_OF_OK;
    adpoll_thread_mgr_t *tmgr = NULL;

    /* find or create a poll thread */
    status = cc_find_or_create_rw_pollthr(&tmgr);

    if(status < 0) {
        CC_LOG_ERROR("%s(%d): %s", __FUNCTION__, __LINE__, 
                     cc_of_strerror(status));
        return status;
    } else {
	    /* add the fd to the thr */
	    // retval??
	    adp_thr_mgr_add_del_fd(tmgr, thr_msg);
	
	    /* add fd to global structures */
	    status = atomic_add_upd_htbls_with_rwsocket(thr_msg->fd, tmgr, key, 
                                                        layer4_proto, ofchann_key);
	    if (status < 0) {
	        CC_LOG_ERROR("%s(%d): %s", __FUNCTION__, __LINE__, 
                         cc_of_strerror(status));
	        /* Del fd from thr_mgr if update of global structures fails */
            cc_del_sockfd_rw_pollthr(tmgr, thr_msg); 
	        return status;
	    }
    }

    return status;
}
