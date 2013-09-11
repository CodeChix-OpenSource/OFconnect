/*-----------------------------------------------------------------------------*/
/* Copyright: CodeChix Bay Area Chapter 2013                                   */
/*-----------------------------------------------------------------------------*/
#include "cc_of_util.h"
/*-----------------------------------------------------------------------*/
/* Utilities to manage the global hash tables                            */
/*-----------------------------------------------------------------------*/


guint cc_ofdev_hash_func(gconstpointer key)
{
    cc_ofdev_key_t *dev_key;
    guint hash = 0;

    dev_key = (cc_ofdev_key_t *)key;
    hash = dev_key->controller_ip_addr + dev_key->switch_ip_addr +
           dev_key->controller_L4_port;
    hash = (hash * 2654435761) % MAX_OFDEV_HTBL_SIZE;
    return hash;
}

guint cc_ofchann_hash_func(gconstpointer key)
{
    cc_ofchannel_key_t *ofchann_key;
    guint hash = 0;

    ofchann_key = (cc_ofchannel_key_t *)key;
    hash = ofchann_key->dp_id + ofchann_key->aux_id;
    hash = (hash * 2654435761) % MAX_OFCHANN_HTBL_SIZE;
    return hash;
}

guint cc_ofrw_hash_func(gconstpointer key)
{
    cc_ofrw_key_t *ofrw_key;
    guint hash = 0;

    ofrw_key = (cc_ofrw_key_t *)key;
    hash = ofrw_key->rw_sockfd;
    hash = (hash * 2654435761) % MAX_OFRW_HTBL_SIZE;
    return hash;
}

gboolean cc_ofdev_htbl_equal_func(gconstpointer a, gconstpointer b)
{
    cc_ofdev_key_t *a_dev, *b_dev;
    a_dev = (cc_ofdev_key_t *)a;
    b_dev = (cc_ofdev_key_t *)b;
    CC_LOG_DEBUG("%s: a controller ip addr 0x%x b is 0x%x",
                 __FUNCTION__,
                a_dev->controller_ip_addr,
                b_dev->controller_ip_addr);
    
    if ((a_dev->controller_ip_addr == b_dev->controller_ip_addr) && 
	    (a_dev->switch_ip_addr == b_dev->switch_ip_addr) &&
        (a_dev->controller_L4_port == b_dev->controller_L4_port)) {
	    return TRUE;
    } else {
	    return FALSE;
    }
}

void cc_of_destroy_generic(gpointer data)
{
    g_free(data);
}

void cc_ofdev_htbl_destroy_val(gpointer data)
{
    cc_ofdev_info_t *dev_info_p;
    dev_info_p = (cc_ofdev_info_t *)data;
    g_mutex_lock(&dev_info_p->ofrw_socket_list_lock);
    g_list_free_full(dev_info_p->ofrw_socket_list,
                     cc_of_destroy_generic);
    g_mutex_unlock(&dev_info_p->ofrw_socket_list_lock);
    
    g_mutex_clear(&dev_info_p->ofrw_socket_list_lock);
    
    g_free(data);
}


gboolean cc_ofchannel_htbl_equal_func(gconstpointer a, gconstpointer b)
{
    cc_ofchannel_key_t *a_chan, *b_chan;
    a_chan = (cc_ofchannel_key_t *)a;
    b_chan = (cc_ofchannel_key_t *)b;

    if ((a_chan->dp_id == b_chan->dp_id) &&
        (a_chan->aux_id == b_chan->aux_id)) {
         return TRUE;
     } else {
         return FALSE;
    }
}


gboolean cc_ofrw_htbl_equal_func(gconstpointer a, gconstpointer b)
{
    cc_ofrw_key_t *a_rw, *b_rw;
    a_rw = (cc_ofrw_key_t *)a;
    b_rw = (cc_ofrw_key_t *)b;

    CC_LOG_DEBUG("%s: a rw_sockfd %d b is %d",
                 __FUNCTION__,
                 a_rw->rw_sockfd,
                 b_rw->rw_sockfd);
    
    if (a_rw->rw_sockfd == b_rw->rw_sockfd) {
	    return TRUE;
    } else {
	    return FALSE;
    }
}

    
cc_of_ret
update_global_htbl(htbl_type_e htbl_type,
                   htbl_update_ops_e htbl_op,
                   gpointer htbl_key,
                   gpointer htbl_data,
                   gboolean *new_entry)
{
    GHashTable *cc_htbl;
    GMutex *cc_htbl_lock;
    gpointer key, info_data;

    *new_entry = FALSE;
    guint old_count;


    switch(htbl_type) {
      case OFDEV:
        cc_htbl = cc_of_global.ofdev_htbl;
        cc_htbl_lock = &cc_of_global.ofdev_htbl_lock;
        g_mutex_lock(cc_htbl_lock);        
        if ((htbl_op == ADD) &&
            (g_hash_table_contains(cc_htbl, htbl_key))) {
            htbl_op = UPD;
            /* create a new key. the insert operation will free this */
            key = malloc(sizeof(cc_ofdev_key_t));
            memcpy(key, htbl_key, sizeof(cc_ofdev_key_t));
            info_data = malloc(sizeof(cc_ofdev_info_t));
            memcpy(info_data, htbl_data, sizeof(cc_ofdev_info_t));
        }
            print_ofdev_htbl();
        break;
      case OFRW:
        cc_htbl = cc_of_global.ofrw_htbl;
        cc_htbl_lock = &cc_of_global.ofrw_htbl_lock;
        g_mutex_lock(cc_htbl_lock);
        if ((htbl_op == ADD) &&
            (g_hash_table_contains(cc_htbl, htbl_key))) {
            htbl_op = UPD;
            /* create a new key. the insert operation will free this */
            key = malloc(sizeof(cc_ofrw_key_t));
            memcpy(key, htbl_key, sizeof(cc_ofrw_key_t));
            info_data = malloc(sizeof(cc_ofrw_info_t));
            memcpy(info_data, htbl_data, sizeof(cc_ofrw_info_t));
        }
        break;
      case OFCHANN:
        cc_htbl = cc_of_global.ofchannel_htbl;
        cc_htbl_lock = &cc_of_global.ofchannel_htbl_lock;
        g_mutex_lock(cc_htbl_lock);
        if ((htbl_op == ADD) &&
            (g_hash_table_contains(cc_htbl, htbl_key))) {
            /* create a new key. the insert operation will free this */
            key = malloc(sizeof(cc_ofchannel_key_t));
            memcpy(key, htbl_key, sizeof(cc_ofchannel_key_t));
            info_data = malloc(sizeof(cc_ofchannel_info_t));
            memcpy(info_data, htbl_data, sizeof(cc_ofchannel_info_t));
        }
        break;
      default:
        CC_LOG_ERROR("%s(%d): invalid htbl type %d",
                     __FUNCTION__, __LINE__, htbl_type);
    }

    if (htbl_op == ADD) {

        old_count = g_hash_table_size(cc_htbl);
        CC_LOG_DEBUG("%s(%d): insert operation for htbl_type %s",
                     __FUNCTION__, __LINE__,
                     (htbl_type == OFDEV)? "OFDEV":
                     ((htbl_type == OFRW)?"OFRW":"OFCHANN"));
        if (htbl_type == OFDEV) {
            CC_LOG_DEBUG("%s(%d) key has controller ip 0x%x, switch ip 0x%x and port %d",
                         __FUNCTION__, __LINE__,
                         ((cc_ofdev_key_t *)htbl_key)->controller_ip_addr,
                         ((cc_ofdev_key_t *)htbl_key)->switch_ip_addr,
                         ((cc_ofdev_key_t *)htbl_key)->controller_L4_port);
        } else if (htbl_type == OFRW) {
            CC_LOG_DEBUG("%s(%d) insert key has socket %d",
                         __FUNCTION__, __LINE__,
                         ((cc_ofrw_key_t *)htbl_key)->rw_sockfd);

            CC_LOG_DEBUG("%s(%d) insert info has l4 proto %s",
                         __FUNCTION__, __LINE__,
                         (((cc_ofrw_info_t *)htbl_data)->layer4_proto == TCP)?
                         "TCP":((((cc_ofrw_info_t *)htbl_data)->layer4_proto
                                 == UDP)? "UDP":"INVALID"));

            if (g_hash_table_contains(cc_htbl, htbl_key)) {
                CC_LOG_DEBUG("(%s(%d): OFRW htbl already contains %d",
                             __FUNCTION__, __LINE__,
                             ((cc_ofrw_key_t *)htbl_key)->rw_sockfd);
            }
        }

        g_hash_table_insert(cc_htbl, htbl_key, htbl_data);

        if (htbl_type == OFDEV) {
            print_ofdev_htbl();
        } else if (htbl_type == OFRW) {
            gpointer rwht_key = NULL, rwht_info = NULL;
            if (g_hash_table_contains(cc_htbl, htbl_key)) {
                CC_LOG_DEBUG("(%s(%d): OFRW htbl contains %d",
                             __FUNCTION__, __LINE__,
                             ((cc_ofrw_key_t *)htbl_key)->rw_sockfd);
            }
            CC_LOG_DEBUG("OFRW htbl size after insert of %d: %d",
                         ((cc_ofrw_key_t *)htbl_key)->rw_sockfd,
                         g_hash_table_size(cc_htbl));
            print_ofrw_htbl();

            if (g_hash_table_lookup_extended(cc_of_global.ofrw_htbl,
                                             htbl_key,
                                             &rwht_key, &rwht_info)
                == FALSE) {
                CC_LOG_DEBUG("extended lookup failed");
            } else {
                cc_ofrw_key_t *rw_key =  NULL;
                cc_ofrw_info_t *rw_info = NULL;
                rw_key = (cc_ofrw_key_t *)rwht_key;
                rw_info = (cc_ofrw_info_t *)rwht_info;
                CC_LOG_DEBUG("extended lookup passed");
                CC_LOG_INFO("key: rw_sockfd: %d "
                            "info: layer4_proto: %s "
                            "info: poll thread name: %s"
                            "info: devkey controller ip: 0x%x"
                            "info: devkey switch ip: 0x%x"
                            "info: devkey l4port: %d",
                            rw_key->rw_sockfd,
                            (rw_info->layer4_proto == TCP)? "TCP":"UDP",
                            rw_info->thr_mgr_p->tname,
                            rw_info->dev_key.controller_ip_addr,
                            rw_info->dev_key.switch_ip_addr,
                            rw_info->dev_key.controller_L4_port);
                
            }
            CC_LOG_DEBUG("%s(%d) key has socket %d",
                         __FUNCTION__, __LINE__,
                         ((cc_ofrw_key_t *)htbl_key)->rw_sockfd);
        } 
        
        if (g_hash_table_size(cc_htbl) > old_count) {
            *new_entry = TRUE;
        }
        
    } else if (htbl_op == DEL) {
        CC_LOG_DEBUG("%s(%d).. HTBL DEL ", __FUNCTION__, __LINE__);
        if (g_hash_table_contains(cc_htbl, htbl_key)) {
            CC_LOG_DEBUG("%s(%d) DEL operation found entry",
                         __FUNCTION__, __LINE__);
        }
        if (g_hash_table_remove(cc_htbl, htbl_key) == FALSE) {
            CC_LOG_DEBUG("%s(%d) DEL unsuccessful",
                         __FUNCTION__, __LINE__);
            
            g_mutex_unlock(cc_htbl_lock);            
            return CC_OF_EHTBL;
        }

    } else if (htbl_op == UPD) {
        old_count = g_hash_table_size(cc_htbl);
        CC_LOG_DEBUG("%s(%d): replace existing entry operation", __FUNCTION__, __LINE__);
        if (htbl_type == OFDEV) {
            CC_LOG_DEBUG("%s(%d) key has controller ip 0x%x, switch ip 0x%x and port %d",
                         __FUNCTION__, __LINE__,
                         ((cc_ofdev_key_t *)key)->controller_ip_addr,
                         ((cc_ofdev_key_t *)key)->switch_ip_addr,
                         ((cc_ofdev_key_t *)key)->controller_L4_port);
        }
        g_hash_table_insert(cc_htbl, key, info_data);
        print_ofdev_htbl();
        print_ofrw_htbl();

        if (htbl_type == OFRW) {
            gpointer rwht_key = NULL, rwht_info = NULL;
            if (g_hash_table_contains(cc_htbl, htbl_key)) {
                CC_LOG_DEBUG("(%s(%d): OFRW htbl contains %d",
                             __FUNCTION__, __LINE__,
                             ((cc_ofrw_key_t *)htbl_key)->rw_sockfd);
            }
            CC_LOG_DEBUG("OFRW htbl size after insert of %d: %d",
                         ((cc_ofrw_key_t *)htbl_key)->rw_sockfd,
                         g_hash_table_size(cc_htbl));
            
            print_ofrw_htbl();
            
            if (g_hash_table_lookup_extended(cc_of_global.ofrw_htbl,
                                             htbl_key,
                                             &rwht_key, &rwht_info)
                == FALSE) {
                CC_LOG_DEBUG("extended lookup failed");
            } else {
                cc_ofrw_key_t *rw_key =  NULL;
                cc_ofrw_info_t *rw_info = NULL;
                rw_key = (cc_ofrw_key_t *)rwht_key;
                rw_info = (cc_ofrw_info_t *)rwht_info;
                CC_LOG_DEBUG("extended lookup passed");
                CC_LOG_INFO("key: rw_sockfd: %d "
                            "info: layer4_proto: %s "
                            "info: poll thread name: %s"
                            "info: devkey controller ip: 0x%x"
                            "info: devkey switch ip: 0x%x"
                            "info: devkey l4port: %d",
                            rw_key->rw_sockfd,
                            (rw_info->layer4_proto == TCP)? "TCP":"UDP",
                            rw_info->thr_mgr_p->tname,
                            rw_info->dev_key.controller_ip_addr,
                            rw_info->dev_key.switch_ip_addr,
                            rw_info->dev_key.controller_L4_port);
                
            }
        }
        
        if (g_hash_table_size(cc_htbl) > old_count) {
            *new_entry = TRUE;
        }

    }
    
    g_mutex_unlock(cc_htbl_lock);
    return CC_OF_OK;
}

void
print_ofdev_htbl(void)
{
    GHashTableIter ofdev_iter;
    cc_ofdev_key_t *dev_key = NULL;
    cc_ofdev_info_t *dev_info = NULL;
    g_hash_table_iter_init(&ofdev_iter, cc_of_global.ofdev_htbl);
    GList *list_elem = NULL;

    CC_LOG_INFO("Printing ofdev Hash Table");
    if (g_hash_table_iter_next(&ofdev_iter,
                               (gpointer *)&dev_key,
                               (gpointer *)&dev_info)) {
        CC_LOG_INFO("key: controller ip: 0x%x "
                    "key: switch ip: 0x%x "
                    "key: l4 port: %d",
                    dev_key->controller_ip_addr,
                    dev_key->switch_ip_addr,
                    dev_key->controller_L4_port);
    }

    //Iterate through the sockets in dev info
    list_elem = g_list_first(dev_info->ofrw_socket_list);
    while (list_elem) {
        CC_LOG_INFO("sockfd: %d",*(int *)(list_elem->data));
        list_elem = g_list_next(list_elem);
    }
    
}

void
print_ofrw_htbl(void)
{
    GHashTableIter ofrw_iter;
    cc_ofrw_key_t *rw_key = NULL;
    cc_ofrw_info_t *rw_info = NULL;
    gpointer rkey = NULL, rinfo = NULL;
    g_hash_table_iter_init(&ofrw_iter, cc_of_global.ofrw_htbl);
    
    CC_LOG_INFO("Printing ofrw Hash Table");
    if (g_hash_table_iter_next(&ofrw_iter,
                               &rkey, &rinfo)) {
//                               (gpointer *)&rw_key,
//                               (gpointer *)&rw_info)) {
        rw_info = (cc_ofrw_info_t *)rinfo;
        rw_key = (cc_ofrw_key_t *)rkey;
        if (rw_info->thr_mgr_p == NULL) {
            CC_LOG_ERROR("%s(%d): no polling thread for %d",
                         __FUNCTION__, __LINE__, rw_key->rw_sockfd);
        } else {
            CC_LOG_INFO("key: rw_sockfd: %d "
                        "info: layer4_proto: %s "
                        "info: poll thread name: %s"
                        "info: devkey controller ip: 0x%x"
                        "info: devkey switch ip: 0x%x"
                        "info: devkey l4port: %d",
                        rw_key->rw_sockfd,
                        (rw_info->layer4_proto == TCP)? "TCP":"UDP",
                        rw_info->thr_mgr_p->tname,
                        rw_info->dev_key.controller_ip_addr,
                        rw_info->dev_key.switch_ip_addr,
                        rw_info->dev_key.controller_L4_port);
        }
    }
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
                      cc_ofdev_key_t key, struct sockaddr_in *client_addr)
{
    cc_ofrw_key_t *ofrw_key;
    cc_ofrw_info_t *ofrw_info;
    gboolean new_entry;
    cc_of_ret rc;
    CC_LOG_DEBUG("%s(%d)", __FUNCTION__, __LINE__);    
    ofrw_key = (cc_ofrw_key_t *)malloc(sizeof(cc_ofrw_key_t));
    ofrw_info = (cc_ofrw_info_t *)malloc(sizeof(cc_ofrw_info_t));

    ofrw_key->rw_sockfd = add_fd;
    ofrw_info->state = CC_OF_RW_DOWN;
    ofrw_info->thr_mgr_p = thr_mgr_p;
    memcpy(&(ofrw_info->dev_key), &key, sizeof(cc_ofdev_key_t));
    memcpy(&(ofrw_info->client_addr), client_addr, sizeof(struct sockaddr_in));
    ofrw_info->layer4_proto = layer4_proto;

    rc = update_global_htbl(OFRW, ADD,
                            ofrw_key, ofrw_info,
                            &new_entry);

//    g_hash_table_insert(cc_of_global.ofrw_htbl, (gpointer)ofrw_key, 
//                        (gpointer)ofrw_info);
    
    if (!new_entry) {
        free(ofrw_key);
    } else {
        CC_LOG_DEBUG("%s(%d): new entry %d", __FUNCTION__, __LINE__,
                     add_fd);
        print_ofrw_htbl();
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
    gpointer ht_dkey, ht_dinfo;
    cc_of_ret retval;

    CC_LOG_DEBUG("%s(%d) dev controller ip 0x%x, switch ip 0x%x, "
                 "layer4 port %d", __FUNCTION__, __LINE__,
                 key.controller_ip_addr, key.switch_ip_addr,
                 key.controller_L4_port);
    
    print_ofdev_htbl();
    
    if (g_hash_table_lookup_extended(cc_of_global.ofdev_htbl,
                                     &key,
                                     &ht_dkey,
                                     &ht_dinfo) == FALSE) {
        return CC_OF_EINVAL;
    }
    ofdev = (cc_ofdev_info_t *)ht_dinfo;
    g_assert(ofdev != NULL);

    list_elem = (int *)malloc(sizeof(int));
    *list_elem = rwsock;
    
    g_mutex_lock(&ofdev->ofrw_socket_list_lock);
    ofdev->ofrw_socket_list = g_list_append(ofdev->ofrw_socket_list,
                                                  list_elem);
    g_mutex_unlock(&ofdev->ofrw_socket_list_lock);

//    retval = (update_global_htbl(OFDEV, ADD, ht_dkey, &ofdev, &new_entry));
    CC_LOG_DEBUG("%s(%d): updated ofdev add ", __FUNCTION__, __LINE__);
    print_ofdev_htbl();
//    return(update_global_htbl(OFDEV, ADD, ht_dkey, &ofdev, &new_entry));
}

gint
ofrwlist_compare_fd (gconstpointer list_elem, gconstpointer compare_fd)
{
    if (*(int *)list_elem < *(int *)compare_fd) {
        return -1;
    } else if (*(int *)list_elem > *(int *)compare_fd) {        
        return 1;
    } else {
        return 0;
    }
    return 0;
}

cc_of_ret
del_ofdev_rwsocket(cc_ofdev_key_t key, int rwsock)
{
    cc_ofdev_info_t *ofdev = NULL;
    GList *tmp_list = NULL;
    int *tmp_rwsock = NULL;
    gboolean new_entry;
    gpointer ht_key, ht_info;

    if (g_hash_table_lookup_extended(cc_of_global.ofdev_htbl,
                                     &key, &ht_key, &ht_info) == FALSE ) {
        CC_LOG_ERROR("%s(%d): device not found key controller ip 0x%x "
                     "switch ip 0x%x port %d", __FUNCTION__, __LINE__,
                     key.controller_ip_addr, key.switch_ip_addr, key.controller_L4_port);
    }
    ofdev = (cc_ofdev_info_t *)ht_info;
    if (!ofdev) {
        return CC_OF_EINVAL;
    }

    g_mutex_lock(&ofdev->ofrw_socket_list_lock);
    tmp_list = g_list_find_custom(ofdev->ofrw_socket_list, &rwsock,
                                  ofrwlist_compare_fd);
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
//(update_global_htbl(OFDEV, ADD, &key, &ofdev, &new_entry));    
    return CC_OF_OK;
}

cc_of_ret
atomic_add_upd_htbls_with_rwsocket(int sockfd, struct sockaddr_in *client_addr,
                                   adpoll_thread_mgr_t  *thr_mgr, 
                                   cc_ofdev_key_t key, L4_type_e layer4_proto,
                                   cc_ofchannel_key_t ofchann_key)
{
    cc_of_ret status = CC_OF_OK;


    if((status = add_upd_ofrw_rwsocket(sockfd, thr_mgr, layer4_proto, key, 
                                       client_addr)) < 0) {
        CC_LOG_ERROR("%s(%d): %s", __FUNCTION__, __LINE__, cc_of_strerror(status));
        return status;
    }

    CC_LOG_DEBUG("%s(%d)....++++...",__FUNCTION__, __LINE__);    
    print_ofrw_htbl();
    print_ofdev_htbl();
    
    if ((status = add_ofdev_rwsocket(key, sockfd)) < 0) {
        CC_LOG_ERROR("%s(%d): %s", __FUNCTION__, __LINE__,
                     cc_of_strerror(status));
        del_ofrw_rwsocket(sockfd);
        return status;
    }
    CC_LOG_DEBUG("%s(%d): successfully updated ofdevice", __FUNCTION__,
                 __LINE__);
    print_ofdev_htbl();
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

	CC_LOG_DEBUG("%s(%d): Atomically added sockfd %d to ofrw & ofdev"
                     "htbls", __FUNCTION__, __LINE__, sockfd);
    
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

    g_sprintf(tname,"rwthr_%d", cc_get_count_rw_pollthr() + 1);

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
    
    *tmgr = (adpoll_thread_mgr_t *)(elem->data);
    
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
    gpointer rwht_key, rwht_info;
    
    CC_LOG_DEBUG("%s(%d) Thread: %s, fd: %d, type: %s", __FUNCTION__,
                 __LINE__, tmgr->tname, thr_msg->fd,
                 (thr_msg->fd_type == PIPE)? "pipe":"socket");

    if (thr_msg == NULL) {
        CC_LOG_ERROR("%s(%d): invalid parameters",
                     __FUNCTION__, __LINE__);
        return CC_OF_EINVAL;
    }
    if (thr_msg->fd_action != DELETE_FD) {
        CC_LOG_ERROR("%s(%d): incorrect action request",
                     __FUNCTION__, __LINE__);
        return CC_OF_EINVAL;
    }

    /* Dummy udp sockfds do not belong to any tmgr 
     */
    if (tmgr) {
        adp_thr_mgr_add_del_fd(tmgr, thr_msg);
    
        if (cc_get_count_rw_pollthr() == 1) {
            // no sorting required with only 1 thread 
            CC_LOG_DEBUG("%s(%d): only one poll thread. skip sorting",
                         __FUNCTION__, __LINE__);

        } else {
            
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
        }
    }
    
    rwkey.rw_sockfd = thr_msg->fd;
    if (g_hash_table_contains(cc_of_global.ofrw_htbl, &rwkey)) {
        CC_LOG_DEBUG("%s(%d) - ofrw_htbl does contain %d",
                     __FUNCTION__, __LINE__, rwkey.rw_sockfd);
    }

    if (g_hash_table_lookup_extended(cc_of_global.ofrw_htbl, &rwkey,
                                     &rwht_key, &rwht_info) == FALSE) {
        CC_LOG_DEBUG("%s(%d) ofrw_htbl does not have the entry for fd %d",
                     __FUNCTION__, __LINE__, rwkey.rw_sockfd);
    }
    rwinfo_p = (cc_ofrw_info_t *)rwht_info;
    
//    rwinfo_p = g_hash_table_lookup(cc_of_global.ofrw_htbl, &rwkey);

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
        CC_LOG_DEBUG("%s(%d): adding fd %d to thread %s",
                     __FUNCTION__, __LINE__, thr_msg->fd,
                     tmgr->tname);
        adp_thr_mgr_add_del_fd(tmgr, thr_msg);

        print_ofdev_htbl();
        CC_LOG_DEBUG("%s(%d): succesfully added fd %d to thread",
                     __FUNCTION__, __LINE__, thr_msg->fd);
        /* add fd to global structures */
        status = atomic_add_upd_htbls_with_rwsocket(thr_msg->fd, NULL,
                                                    tmgr,
                                                    key, 
                                                    layer4_proto,
                                                    ofchann_key);
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
