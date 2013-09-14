#include "cc_of_global.h"
#include "cc_of_priv.h"
#include "string.h"

cc_of_global_t cc_of_global;
// Does this need a lock for the global struct ? 

extern net_svcs_t tcp_sockfns;
extern net_svcs_t udp_sockfns;


inline const char *cc_of_strerror(int errnum)
{
    if (errnum > 0) {
	    return "Invalid errnum";
    } else if (errnum <= -(int)CC_OF_ERRTABLE_SIZE){
	    return "Unknown error";
    } else {
	    return cc_of_errtable[-errnum];
    }
}


cc_of_ret
cc_of_lib_init(of_dev_type_e dev_type)
{
    cc_of_ret status = CC_OF_OK;
    CC_LOG_DEBUG("%s(%d): %s", __FUNCTION__, __LINE__,
                 "Started Registering OFLIB");
 
    // Initialize cc_of_global
    cc_of_global.ofdebug_enable = FALSE;
    cc_of_global.oflog_enable = FALSE;
    cc_of_global.ofut_enable = FALSE;
    cc_of_global.oflog_fd = NULL;
    cc_of_global.oflog_file = malloc(sizeof(char) *
                                     LOG_FILE_NAME_SIZE);
    g_mutex_init(&cc_of_global.oflog_lock);
    
    cc_of_global.ofdev_type = dev_type;
    cc_of_global.ofdev_htbl = g_hash_table_new_full(cc_ofdev_hash_func,
                                                    cc_ofdev_htbl_equal_func,
                                                    cc_of_destroy_generic,
                                                    cc_ofdev_htbl_destroy_val);
    if (cc_of_global.ofdev_htbl == NULL) {
	    status = CC_OF_EHTBL;
	    cc_of_lib_free();

	    CC_LOG_FATAL("%s(%d): %s", __FUNCTION__, __LINE__,
                     cc_of_strerror(status));
    }
    g_mutex_init(&cc_of_global.ofdev_htbl_lock);

    cc_of_global.ofchannel_htbl = g_hash_table_new_full(cc_ofchann_hash_func,
                                                        cc_ofchannel_htbl_equal_func,
                                                        cc_of_destroy_generic,
                                                        cc_of_destroy_generic);
    if (cc_of_global.ofchannel_htbl == NULL) {
	    status = CC_OF_EHTBL;
	    cc_of_lib_free();
	    CC_LOG_FATAL("%s(%d): %s", __FUNCTION__, __LINE__,
                     cc_of_strerror(status));
    }
    g_mutex_init(&cc_of_global.ofchannel_htbl_lock);

    cc_of_global.ofrw_htbl = g_hash_table_new_full(cc_ofrw_hash_func,
                                                   cc_ofrw_htbl_equal_func,
                                                   cc_of_destroy_generic,
                                                   cc_of_destroy_generic);

    if (cc_of_global.ofrw_htbl == NULL) {
	    status = CC_OF_EHTBL;
	    cc_of_lib_free();
	    CC_LOG_FATAL("%s(%d): %s", __FUNCTION__, __LINE__,
                     cc_of_strerror(status));
    }
    g_mutex_init(&cc_of_global.ofrw_htbl_lock);

    cc_of_global.NET_SVCS[TCP] = tcp_sockfns;
    cc_of_global.NET_SVCS[UDP] = udp_sockfns;

    cc_of_global.oflisten_pollthr_p = adp_thr_mgr_new("oflisten_thr",
                                                      MAX_PER_THREAD_RWSOCKETS,
                                                      MAX_PER_THREAD_PIPES);
    if (cc_of_global.oflisten_pollthr_p == NULL) {
	    status = CC_OF_EMISC;
	    cc_of_lib_free();
	    CC_LOG_FATAL("%s(%d): %s", __FUNCTION__, __LINE__,
                     cc_of_strerror(status));
    }
    CC_LOG_DEBUG("%s(%d): %s", __FUNCTION__, __LINE__,
                 "CREATED POLLTHR FOR listen sockets");

    cc_of_global.ofrw_pollthr_list = NULL;
    g_mutex_init(&cc_of_global.ofrw_pollthr_list_lock);
    adpoll_thread_mgr_t *tmgr = NULL;
    if ((status = cc_create_rw_pollthr(&tmgr)) < 0) {
	    cc_of_lib_free();
	    CC_LOG_FATAL("%s(%d): %s", __FUNCTION__, __LINE__, 
                     cc_of_strerror(status));
    }
    CC_LOG_DEBUG("%s(%d): %s", __FUNCTION__, __LINE__,
                 "CREATED POLLTHR FOR rwsockets");
    CC_LOG_INFO("%s(%d): %s", __FUNCTION__, __LINE__,
                "CC_OF_Library initilaized successfully");
    return status;
}


cc_of_ret
cc_of_lib_free()
{
    cc_of_ret status = CC_OF_OK;
    GList *elem;

    CC_LOG_DEBUG("%s(%d): %s", __FUNCTION__, __LINE__,
                 "Started Freeing OFLIB");

    if (cc_of_global.ofdev_htbl) {
        GHashTableIter ofdev_iter;
        cc_ofdev_key_t *dev_key;
        cc_ofdev_info_t *dev_info;

        g_mutex_lock(&cc_of_global.ofdev_htbl_lock);
        g_hash_table_iter_init(&ofdev_iter, cc_of_global.ofdev_htbl);
        while (g_hash_table_iter_next(&ofdev_iter, (gpointer *)&dev_key, 
                                      (gpointer *)&dev_info)) {
            status = cc_of_dev_free_lockfree(dev_key->controller_ip_addr, 
                                             dev_key->switch_ip_addr,
                                             dev_key->controller_L4_port);
            if (status < 0) {
                CC_LOG_ERROR("%s(%d): %s, Error while freeing a dev in ofdev_htbl",
                             __FUNCTION__, __LINE__, cc_of_strerror(status));
            }
        }
        g_mutex_unlock(&cc_of_global.ofdev_htbl_lock);
    }

    g_mutex_clear(&cc_of_global.ofdev_htbl_lock);

    /* Cleaning up all devices should have cleaned both
     * ofchannel and ofrw htbls as well. But, cleanup again 
     * if anything is remaining in these htbls.
     */
    g_mutex_lock(&cc_of_global.ofrw_htbl_lock);
    g_mutex_lock(&cc_of_global.ofchannel_htbl_lock);

    g_hash_table_destroy(cc_of_global.ofchannel_htbl);
    g_hash_table_destroy(cc_of_global.ofrw_htbl);
        
    g_mutex_clear(&cc_of_global.ofchannel_htbl_lock);
    g_mutex_clear(&cc_of_global.ofrw_htbl_lock);
    
    g_mutex_unlock(&cc_of_global.ofchannel_htbl_lock);    
    g_mutex_unlock(&cc_of_global.ofrw_htbl_lock);

    if (cc_of_global.oflisten_pollthr_p)
        adp_thr_mgr_free(cc_of_global.oflisten_pollthr_p);
    
    g_mutex_lock(&cc_of_global.ofrw_pollthr_list_lock);
    elem = g_list_first(cc_of_global.ofrw_pollthr_list);
    while (elem != NULL) {
        adp_thr_mgr_free((adpoll_thread_mgr_t *)elem->data);
        elem = elem->next;
    }
    g_list_free_full(cc_of_global.ofrw_pollthr_list,
                     cc_of_destroy_generic);
    g_mutex_unlock(&cc_of_global.ofrw_pollthr_list_lock);
    g_mutex_clear(&cc_of_global.ofrw_pollthr_list_lock);

    /* clear the logging last */
    cc_of_log_toggle(FALSE);
    free(cc_of_global.oflog_file);
    g_mutex_clear(&cc_of_global.oflog_lock);
    return CC_OF_OK;
}


cc_of_ret
cc_of_dev_register(uint32_t controller_ipaddr, 
                   uint32_t switch_ipaddr, 
                   uint16_t controller_L4_port,
                   cc_ofver_e max_ofver, 
                   cc_of_recv_pkt recv_func,
                   cc_of_accept_channel accept_func,
                   cc_of_delete_channel del_func)
{
    cc_of_ret status = CC_OF_OK;
    cc_ofdev_key_t *key;
    gpointer ht_dev_key, ht_dev_info;
    cc_ofdev_info_t *dev_info;
    char switch_ip[INET_ADDRSTRLEN];
    char controller_ip[INET_ADDRSTRLEN]; 
    ipaddr_v4v6_t controller_ip_addr = (ipaddr_v4v6_t)controller_ipaddr;
    ipaddr_v4v6_t switch_ip_addr = (ipaddr_v4v6_t)switch_ipaddr;

    CC_LOG_DEBUG("%s(%d): %s", __FUNCTION__, __LINE__,
                 "Started Registering Device");
 
    if (max_ofver >= MAX_OFVER_TYPE) {
        CC_LOG_ERROR("%s(%d): max openflow version invalid", 
                     __FUNCTION__, __LINE__);
        return CC_OF_EINVAL;
    }
   
    key = g_malloc0(sizeof(cc_ofdev_key_t));
    if (key == NULL) {
	    status = CC_OF_ENOMEM;
	    CC_LOG_ERROR("%s(%d): %s", __FUNCTION__, __LINE__,
                     cc_of_strerror(status));
	    return status;
    }

    key->controller_ip_addr = controller_ip_addr;
    key->switch_ip_addr = switch_ip_addr;
    key->controller_L4_port = controller_L4_port;

    inet_ntop(AF_INET, &key->switch_ip_addr, switch_ip, sizeof(switch_ip));
    inet_ntop(AF_INET, &key->controller_ip_addr, controller_ip,
              sizeof(controller_ip));
    
    dev_info = g_malloc0(sizeof(cc_ofdev_info_t));
    if (dev_info == NULL) {
        status = CC_OF_ENOMEM;
        CC_LOG_ERROR("%s(%d): %s", __FUNCTION__, __LINE__,
                     cc_of_strerror(status));
        g_free(key);
        return status;
    }

    dev_info->of_max_ver = max_ofver;
    dev_info->ofrw_socket_list = NULL;

    dev_info->recv_func = recv_func;
    dev_info->accept_chann_func = accept_func;
    dev_info->del_chann_func = del_func; 

    // Add this new device entry to ofdev_htbl
    g_mutex_lock(&cc_of_global.ofdev_htbl_lock);
    g_hash_table_insert(cc_of_global.ofdev_htbl, (gpointer)key, 
                        (gpointer)dev_info);


    if (g_hash_table_lookup_extended(cc_of_global.ofdev_htbl,
                                     key,
                                     &ht_dev_key,
                                     &ht_dev_info) == FALSE) {
        CC_LOG_ERROR("%s(%d): lookup returned FALSE", __FUNCTION__,
                     __LINE__);
    } else {
        g_assert(ht_dev_key != NULL);
        g_assert(ht_dev_info != NULL);
        
        CC_LOG_DEBUG("%s(%d): actual key returned 0x%x ip address",
                     __FUNCTION__, __LINE__,
                     ((cc_ofdev_key_t *)ht_dev_key)->controller_ip_addr);
    }
    g_mutex_unlock(&cc_of_global.ofdev_htbl_lock);
    
    if (cc_of_global.ofdev_type == CONTROLLER) {

        // Create a TCP sockfd for tcp connections
        dev_info->main_sockfd_tcp =
            cc_of_global.NET_SVCS[TCP].open_serverfd(*key);
        if (dev_info->main_sockfd_tcp < 0) {
	        status = CC_OF_EMISC;
	        CC_LOG_ERROR("%s(%d): %s", __FUNCTION__, __LINE__,
                         cc_of_strerror(status));
            g_free(key);
            g_free(dev_info);
	        return status;
        }
        CC_LOG_DEBUG("%s(%d): %s", __FUNCTION__, __LINE__,
                 "Created TCP listenfd");

        // Create a udp sockfd for udp connections

        dev_info->main_sockfd_udp =
            cc_of_global.NET_SVCS[UDP].open_serverfd(*key);
        if (dev_info->main_sockfd_udp < 0) {
	        status = CC_OF_EMISC;
	        CC_LOG_ERROR("%s(%d): %s", __FUNCTION__, __LINE__,
                         cc_of_strerror(status));
            g_free(key);
            g_free(dev_info);
	        return status;
        }

        CC_LOG_DEBUG("%s(%d): %s", __FUNCTION__, __LINE__,
                 "Created UDP serverfd");
 
    }

#if 0
    //add this before the tcp and udp 
    // Add this new device entry to ofdev_htbl
    g_mutex_lock(&cc_of_global.ofdev_htbl_lock);
    g_hash_table_insert(cc_of_global.ofdev_htbl, (gpointer)key, 
                        (gpointer)dev_info);


    if (g_hash_table_lookup_extended(cc_of_global.ofdev_htbl,
                                     key,
                                     &ht_dev_key,
                                     &ht_dev_info) == FALSE) {
        CC_LOG_ERROR("%s(%d): lookup returned FALSE", __FUNCTION__,
                     __LINE__);
    } else {
        g_assert(ht_dev_key != NULL);
        g_assert(ht_dev_info != NULL);
        
        CC_LOG_DEBUG("%s(%d): actual key returned 0x%x ip address",
                     __FUNCTION__, __LINE__,
                     ((cc_ofdev_key_t *)ht_dev_key)->controller_ip_addr);
    }
    g_mutex_unlock(&cc_of_global.ofdev_htbl_lock);
#endif
    CC_LOG_INFO("%s(%d): %s , controllerIP:%s, switchIP:%s,"
                "controllerPort:%hu",__FUNCTION__, __LINE__, 
                "CC_OF_DEV initilaized successfully", 
                controller_ip, switch_ip, controller_L4_port);

    return status;
}


cc_of_ret
cc_of_dev_free(uint32_t controller_ip_addr,
               uint32_t switch_ip_addr,
               uint16_t controller_L4_port)
{
    cc_of_ret status = CC_OF_OK;
    cc_ofdev_key_t *dkey, *ht_dkey = NULL;
    gpointer ht_dev_key, ht_dev_info;
    cc_ofdev_info_t *ht_dinfo = NULL;
    char switch_ip[INET_ADDRSTRLEN];
    char controller_ip[INET_ADDRSTRLEN];
    adpoll_thr_msg_t thr_msg;
    GList *elem = NULL;
    gboolean new_entry;
    adpoll_thread_mgr_t *tmgr = NULL;

    dkey = g_malloc0(sizeof(cc_ofdev_key_t));    
    dkey->controller_ip_addr = controller_ip_addr;
    dkey->switch_ip_addr = switch_ip_addr;
    dkey->controller_L4_port = controller_L4_port;

    inet_ntop(AF_INET, &dkey->switch_ip_addr, switch_ip, sizeof(switch_ip));
    inet_ntop(AF_INET, &dkey->controller_ip_addr, controller_ip, 
              sizeof(controller_ip));

    g_mutex_lock(&cc_of_global.ofdev_htbl_lock);
    g_mutex_lock(&cc_of_global.ofchannel_htbl_lock);
    g_mutex_lock(&cc_of_global.ofrw_htbl_lock);
 
    g_assert(g_hash_table_size(cc_of_global.ofdev_htbl) == 1);
    print_ofdev_htbl();

    g_assert(g_hash_table_contains(cc_of_global.ofdev_htbl,
                                   dkey) == TRUE);

    CC_LOG_DEBUG("%s(%d): looking up "
                 "controller_ip-0x%x, switch_ip-0x%x,"
                 "controller_l4_port-%d",__FUNCTION__, __LINE__,
                 dkey->controller_ip_addr,
                 dkey->switch_ip_addr,
                 dkey->controller_L4_port);

    print_ofdev_htbl();    
    if (g_hash_table_lookup_extended(cc_of_global.ofdev_htbl,
                                     dkey,
                                     &ht_dev_key,
                                     &ht_dev_info) == FALSE) {
        if (ht_dev_key != NULL) {
            CC_LOG_DEBUG("act dev key controller ip is 0x%x",
                         ((cc_ofdev_key_t *)ht_dev_key)->controller_ip_addr);
        }
        CC_LOG_ERROR("%s(%d): lookup returned FALSE", __FUNCTION__,
                     __LINE__);
    }
    if (ht_dev_info == NULL) {
        CC_LOG_ERROR("%s(%d):, could not find ofdev_info in ofdev_htbl"
                     "for dev controller_ip-%s, switch_ip-%s,"
                     "controller_l4_port-%hu",__FUNCTION__, __LINE__,
                     controller_ip, switch_ip, controller_L4_port);
        g_mutex_unlock(&cc_of_global.ofrw_htbl_lock);
        g_mutex_unlock(&cc_of_global.ofchannel_htbl_lock);
        g_mutex_unlock(&cc_of_global.ofdev_htbl_lock);
        return CC_OF_EINVAL;
    }


    ht_dkey = (cc_ofdev_key_t *)ht_dev_key;
    ht_dinfo = (cc_ofdev_info_t *)ht_dev_info;

    // close all ofchannels for this device
    elem = g_list_first(ht_dinfo->ofrw_socket_list);
    if (elem == NULL) {
        CC_LOG_DEBUG("%s(%d): no sockets in ofrw socket list", __FUNCTION__, 
                     __LINE__);
    }

    while (elem != NULL) {
        cc_ofrw_key_t rwkey;
        cc_ofrw_info_t *rwinfo;
        gpointer rwht_key = NULL, rwht_info = NULL;

        rwkey.rw_sockfd = *((int *)(elem->data));
        CC_LOG_DEBUG("%s %d here....here", __FUNCTION__, __LINE__);
        CC_LOG_DEBUG("rw sockfd found: %d", rwkey.rw_sockfd);

        print_ofrw_htbl();

        if (g_hash_table_lookup_extended(cc_of_global.ofrw_htbl, &rwkey,
                                         &rwht_key, &rwht_info) == FALSE) {
            CC_LOG_ERROR("%s(%d): ofrw lookup failed", __FUNCTION__,
                         __LINE__);
        } else {
            CC_LOG_DEBUG("%s(%d): found %d in ofrw htbl", __FUNCTION__,
                         __LINE__, rwkey.rw_sockfd);
        }
        
        rwinfo = (cc_ofrw_info_t *)rwht_info;


        
        if (rwinfo == NULL) {
            CC_LOG_ERROR("%s(%d): could not find rwsockinfo in ofrw_htbl"
                         "for sockfd-%d", __FUNCTION__, __LINE__, rwkey.rw_sockfd);
            continue;
        } else {

            CC_LOG_DEBUG("%s(%d): found rwsock %d with rwinfo: "
                        "info: layer4_proto: %s "
                        "info: devkey controller ip: 0x%x "
                        "info: devkey switch ip: 0x%x "
                        "info: devkey l4port: %d",
                         __FUNCTION__, __LINE__,
                        rwkey.rw_sockfd,
                        (rwinfo->layer4_proto == TCP)? "TCP":"UDP",
                        rwinfo->dev_key.controller_ip_addr,
                        rwinfo->dev_key.switch_ip_addr,
                        rwinfo->dev_key.controller_L4_port);

        }
                         
        elem = elem->next;
        
       /* the udp port for this device is also closed by this walk */
        status = cc_of_global.NET_SVCS[rwinfo->layer4_proto].close_conn(rwkey.rw_sockfd);

        if (status < 0) {
            CC_LOG_ERROR("%s(%d): %s, Error while closing ofchannel"
                         "sockfd-%d", __FUNCTION__, __LINE__,
                         cc_of_strerror(status), rwkey.rw_sockfd);
        }
    }

    // close main tcp listenfd and remove it from oflisten_pollthr.
    thr_msg.fd = ht_dinfo->main_sockfd_tcp;
    thr_msg.fd_type = SOCKET;
    thr_msg.fd_action = DELETE_FD;

    status = adp_thr_mgr_add_del_fd(cc_of_global.oflisten_pollthr_p, &thr_msg);
    if (status != -1 ) {
        CC_LOG_ERROR("%s(%d):Error deleting tcp_listenfd from oflisten_pollthr_p: %s",
                     __FUNCTION__, __LINE__, cc_of_strerror(errno));
    }
    status = close(ht_dinfo->main_sockfd_tcp);
    if (status < 0) {
        CC_LOG_ERROR("%s(%d):Error closing tcp_listenfd: %s",
                     __FUNCTION__, __LINE__, strerror(errno));
    }

    // delete dev from devhtbl
    status = update_global_htbl_lockfree(OFDEV, DEL, ht_dkey, NULL, &new_entry);
    if (status < 0) {
        CC_LOG_ERROR("%s(%d): %s, Error while freeing dev"
                     "controller_ip-%s, switch_ip-%s,"
                     "controller_l4_port-%hu",
                     __FUNCTION__, __LINE__,
                     cc_of_strerror(status), controller_ip, switch_ip, 
                     controller_L4_port);
        g_mutex_unlock(&cc_of_global.ofrw_htbl_lock);
        g_mutex_unlock(&cc_of_global.ofchannel_htbl_lock);
        g_mutex_unlock(&cc_of_global.ofdev_htbl_lock);
 
        return status;
    }


    CC_LOG_INFO("%s(%d):, Devfree success for device"
                 "controller_ip-%s, switch_ip-%s,"
                 "controller_l4_port-%hu",__FUNCTION__, __LINE__,
                 controller_ip, switch_ip, controller_L4_port);

    g_free(dkey);
    g_mutex_unlock(&cc_of_global.ofrw_htbl_lock);
    g_mutex_unlock(&cc_of_global.ofchannel_htbl_lock);
    g_mutex_unlock(&cc_of_global.ofdev_htbl_lock);
 
    return status;
}


cc_of_ret
cc_of_dev_free_lockfree(uint32_t controller_ip_addr,
                        uint32_t switch_ip_addr,
                        uint16_t controller_L4_port)
{
    cc_of_ret status = CC_OF_OK;
    cc_ofdev_key_t *dkey, *ht_dkey = NULL;
    gpointer ht_dev_key, ht_dev_info;
    cc_ofdev_info_t *ht_dinfo = NULL;
    char switch_ip[INET_ADDRSTRLEN];
    char controller_ip[INET_ADDRSTRLEN];
    adpoll_thr_msg_t thr_msg;
    GList *elem = NULL;
    gboolean new_entry;
    adpoll_thread_mgr_t *tmgr = NULL;

    dkey = g_malloc0(sizeof(cc_ofdev_key_t));    
    dkey->controller_ip_addr = controller_ip_addr;
    dkey->switch_ip_addr = switch_ip_addr;
    dkey->controller_L4_port = controller_L4_port;

    inet_ntop(AF_INET, &dkey->switch_ip_addr, switch_ip, sizeof(switch_ip));
    inet_ntop(AF_INET, &dkey->controller_ip_addr, controller_ip, 
              sizeof(controller_ip));

    //g_mutex_lock(&cc_of_global.ofdev_htbl_lock);
    g_mutex_lock(&cc_of_global.ofchannel_htbl_lock);
    g_mutex_lock(&cc_of_global.ofrw_htbl_lock);
    
    g_assert(g_hash_table_size(cc_of_global.ofdev_htbl) == 1);
    print_ofdev_htbl();
    

    g_assert(g_hash_table_contains(cc_of_global.ofdev_htbl,
                                   dkey) == TRUE);

    CC_LOG_DEBUG("%s(%d): looking up "
                 "controller_ip-0x%x, switch_ip-0x%x,"
                 "controller_l4_port-%d",__FUNCTION__, __LINE__,
                 dkey->controller_ip_addr,
                 dkey->switch_ip_addr,
                 dkey->controller_L4_port);

    print_ofdev_htbl();    
    if (g_hash_table_lookup_extended(cc_of_global.ofdev_htbl,
                                     dkey,
                                     &ht_dev_key,
                                     &ht_dev_info) == FALSE) {
        if (ht_dev_key != NULL) {
            CC_LOG_DEBUG("act dev key controller ip is 0x%x",
                         ((cc_ofdev_key_t *)ht_dev_key)->controller_ip_addr);
        }
        CC_LOG_ERROR("%s(%d): lookup returned FALSE", __FUNCTION__,
                     __LINE__);
    }
    if (ht_dev_info == NULL) {
        CC_LOG_ERROR("%s(%d):, could not find ofdev_info in ofdev_htbl"
                     "for dev controller_ip-%s, switch_ip-%s,"
                     "controller_l4_port-%hu",__FUNCTION__, __LINE__,
                     controller_ip, switch_ip, controller_L4_port);
        //g_mutex_unlock(&cc_of_global.ofdev_htbl_lock);
        g_mutex_unlock(&cc_of_global.ofrw_htbl_lock);
        g_mutex_unlock(&cc_of_global.ofchannel_htbl_lock);
  
        return CC_OF_EINVAL;
    }


    ht_dkey = (cc_ofdev_key_t *)ht_dev_key;
    ht_dinfo = (cc_ofdev_info_t *)ht_dev_info;

    // close all ofchannels for this device
    elem = g_list_first(ht_dinfo->ofrw_socket_list);
    if (elem == NULL) {
        CC_LOG_DEBUG("%s(%d): no sockets in ofrw socket list", __FUNCTION__, 
                     __LINE__);
    }

    while (elem != NULL) {
        cc_ofrw_key_t rwkey;
        cc_ofrw_info_t *rwinfo;
        gpointer rwht_key = NULL, rwht_info = NULL;

        rwkey.rw_sockfd = *((int *)(elem->data));
        CC_LOG_DEBUG("%s %d here....here", __FUNCTION__, __LINE__);
        CC_LOG_DEBUG("rw sockfd found: %d", rwkey.rw_sockfd);

        print_ofrw_htbl();

        if (g_hash_table_lookup_extended(cc_of_global.ofrw_htbl, &rwkey,
                                         &rwht_key, &rwht_info) == FALSE) {
            CC_LOG_ERROR("%s(%d): ofrw lookup failed", __FUNCTION__,
                         __LINE__);
        } else {
            CC_LOG_DEBUG("%s(%d): found %d in ofrw htbl", __FUNCTION__,
                         __LINE__, rwkey.rw_sockfd);
        }
        
        rwinfo = (cc_ofrw_info_t *)rwht_info;


        
        if (rwinfo == NULL) {
            CC_LOG_ERROR("%s(%d): could not find rwsockinfo in ofrw_htbl"
                         "for sockfd-%d", __FUNCTION__, __LINE__, rwkey.rw_sockfd);
            continue;
        } else {

            CC_LOG_DEBUG("%s(%d): found rwsock %d with rwinfo: "
                        "info: layer4_proto: %s "
                        "info: devkey controller ip: 0x%x "
                        "info: devkey switch ip: 0x%x "
                        "info: devkey l4port: %d",
                         __FUNCTION__, __LINE__,
                        rwkey.rw_sockfd,
                        (rwinfo->layer4_proto == TCP)? "TCP":"UDP",
                        rwinfo->dev_key.controller_ip_addr,
                        rwinfo->dev_key.switch_ip_addr,
                        rwinfo->dev_key.controller_L4_port);

        }
                         
        elem = elem->next;

       /* the udp port for this device is also closed by this walk */
        status = cc_of_global.NET_SVCS[rwinfo->layer4_proto].close_conn(rwkey.rw_sockfd);

        if (status < 0) {
            CC_LOG_ERROR("%s(%d): %s, Error while closing ofchannel"
                         "sockfd-%d", __FUNCTION__, __LINE__,
                         cc_of_strerror(status), rwkey.rw_sockfd);
        }
    }

    // close main tcp listenfd and remove it from oflisten_pollthr.
    thr_msg.fd = ht_dinfo->main_sockfd_tcp;
    thr_msg.fd_type = SOCKET;
    thr_msg.fd_action = DELETE_FD;

    status = adp_thr_mgr_add_del_fd(cc_of_global.oflisten_pollthr_p, &thr_msg);
    if (status != -1 ) {
        CC_LOG_ERROR("%s(%d):Error deleting tcp_listenfd from oflisten_pollthr_p: %s",
                     __FUNCTION__, __LINE__, cc_of_strerror(errno));
    }
    status = close(ht_dinfo->main_sockfd_tcp);
    if (status < 0) {
        CC_LOG_ERROR("%s(%d):Error closing tcp_listenfd: %s",
                     __FUNCTION__, __LINE__, strerror(errno));
    }

    // delete dev from devhtbl
    status = update_global_htbl_lockfree(OFDEV, DEL, ht_dkey, NULL, &new_entry);
    if (status < 0) {
        CC_LOG_ERROR("%s(%d): %s, Error while freeing dev"
                     "controller_ip-%s, switch_ip-%s,"
                     "controller_l4_port-%hu",
                     __FUNCTION__, __LINE__,
                     cc_of_strerror(status), controller_ip, switch_ip, 
                     controller_L4_port);
        //g_mutex_unlock(&cc_of_global.ofdev_htbl_lock);
        g_mutex_unlock(&cc_of_global.ofrw_htbl_lock);
        g_mutex_unlock(&cc_of_global.ofchannel_htbl_lock);
        return status;
    }


    CC_LOG_INFO("%s(%d):, Devfree success for device"
                 "controller_ip-%s, switch_ip-%s,"
                 "controller_l4_port-%hu",__FUNCTION__, __LINE__,
                 controller_ip, switch_ip, controller_L4_port);

    g_free(dkey);
    //g_mutex_unlock(&cc_of_global.ofdev_htbl_lock);
    g_mutex_unlock(&cc_of_global.ofrw_htbl_lock);
    g_mutex_unlock(&cc_of_global.ofchannel_htbl_lock);
 
    return status;
}

cc_of_ret cc_of_create_channel(uint32_t controller_ip,
                               uint32_t switch_ip,
                               uint16_t controller_L4_port,
                               uint64_t dp_id, 
                               uint8_t aux_id,
                               L4_type_e l4_proto)
{
   cc_of_ret status = CC_OF_OK;
   cc_ofdev_key_t ofdev_key;
   cc_ofchannel_key_t ofchann_key;
   int rw_sockfd;
   
   if (l4_proto >= MAX_L4_TYPE) {
        CC_LOG_ERROR("%s(%d): l4_proto is invalid", 
                     __FUNCTION__, __LINE__);
        return CC_OF_EINVAL;
   }
 
   ofdev_key.controller_ip_addr = (ipaddr_v4v6_t)controller_ip; 
   ofdev_key.switch_ip_addr = (ipaddr_v4v6_t)switch_ip;
   ofdev_key.controller_L4_port = controller_L4_port;

   ofchann_key.dp_id = dp_id;
   ofchann_key.aux_id = aux_id;

   rw_sockfd = cc_of_global.NET_SVCS[l4_proto].open_clientfd(ofdev_key, ofchann_key);
   if (rw_sockfd < 0) {
       status = CC_OF_EMISC;
       CC_LOG_ERROR("%s(%d): %s, Unable to create new ofchannel dp_id-%lu, aux_id-%u", 
                    __FUNCTION__, __LINE__, cc_of_strerror(status), dp_id, aux_id);
       return status;
   }

   CC_LOG_INFO("%s(%d):, created new ofchannel dp_id-%lu, aux_id-%u",
                __FUNCTION__, __LINE__, dp_id, aux_id);

   return status;
}


cc_of_ret
cc_of_destroy_channel(uint64_t dp_id, uint8_t aux_id)
{
    cc_of_ret status = CC_OF_OK;
    cc_ofchannel_key_t ofchann_key;
    cc_ofchannel_info_t *ofchann_info;
    cc_ofrw_key_t rwkey;
    cc_ofrw_info_t *rwinfo = NULL;
    gpointer chht_key = NULL, chht_info = NULL;
    gpointer rwht_key = NULL, rwht_info = NULL;

    ofchann_key.dp_id = dp_id;
    ofchann_key.aux_id = aux_id;

    g_mutex_lock(&cc_of_global.ofdev_htbl_lock);
    g_mutex_lock(&cc_of_global.ofchannel_htbl_lock);
    
    if (g_hash_table_lookup_extended(cc_of_global.ofchannel_htbl, 
                                     &ofchann_key, &chht_key,
                                     &chht_info) == FALSE) {
        CC_LOG_ERROR("%s(%d):, could not find ofchann_info in ofchannel_htbl"
                     "for key dp_id-%lu, aux_id-%u",__FUNCTION__, __LINE__, 
                     dp_id, aux_id);
        g_mutex_unlock(&cc_of_global.ofchannel_htbl_lock);
        g_mutex_unlock(&cc_of_global.ofdev_htbl_lock);
        return CC_OF_EINVAL;
    }
    
    ofchann_info = (cc_ofchannel_info_t *)chht_info;
        
    rwkey.rw_sockfd = ofchann_info->rw_sockfd;
    g_mutex_lock(&cc_of_global.ofrw_htbl_lock);
    if (g_hash_table_lookup_extended(cc_of_global.ofrw_htbl, &rwkey,
                                     &rwht_key, &rwht_info) == FALSE) {
        CC_LOG_ERROR("%s(%d): could not find rwsockinfo in ofrw_htbl"
                     "for sockfd-%d", __FUNCTION__, __LINE__, rwkey.rw_sockfd);
        g_mutex_unlock(&cc_of_global.ofrw_htbl_lock);
        g_mutex_unlock(&cc_of_global.ofchannel_htbl_lock);
        g_mutex_unlock(&cc_of_global.ofdev_htbl_lock);
        return CC_OF_EINVAL;
    }
    rwinfo = (cc_ofrw_info_t *)rwht_info;

    status = cc_of_global.NET_SVCS[rwinfo->layer4_proto].close_conn(ofchann_info->rw_sockfd);
    if (status < 0) {
        CC_LOG_ERROR("%s(%d): %s, Error while destroying ofchannel"
                     "dp_id-%lu,aux_id-%u", __FUNCTION__, __LINE__, 
                     cc_of_strerror(status), dp_id, aux_id);
        g_mutex_unlock(&cc_of_global.ofrw_htbl_lock);
        g_mutex_unlock(&cc_of_global.ofchannel_htbl_lock);
        g_mutex_unlock(&cc_of_global.ofdev_htbl_lock);
        return status;
    }
    g_mutex_unlock(&cc_of_global.ofrw_htbl_lock);
    g_mutex_unlock(&cc_of_global.ofchannel_htbl_lock);
    g_mutex_unlock(&cc_of_global.ofdev_htbl_lock);
    CC_LOG_INFO("%s(%d):, destroyed ofchannel dp_id-%lu,aux_id-%u", 
                 __FUNCTION__, __LINE__, dp_id, aux_id);

    return status;
}


cc_of_ret
cc_of_send_pkt(uint64_t dp_id, uint8_t aux_id, void *of_msg, 
               size_t msg_len)
{
    cc_ofchannel_info_t *chann_info;
    adpoll_thread_mgr_t *tmgr = NULL;
    int send_rwsock;    
    adpoll_send_msg_t  *msg_p;
    msg_p = (adpoll_send_msg_t *)SEND_MSG_BUF;
    cc_ofchannel_key_t chann_id;
    gpointer chht_key = NULL, chht_info = NULL;

    chann_id.dp_id = dp_id;
    chann_id.aux_id = aux_id;

    if (of_msg == NULL) {
        CC_LOG_ERROR("%s(%d): message is invalid",
                     __FUNCTION__, __LINE__);
        return CC_OF_EINVAL;
    }
    g_mutex_lock(&cc_of_global.ofchannel_htbl_lock);
    if (g_hash_table_lookup_extended(cc_of_global.ofchannel_htbl,
                                     (gconstpointer)&chann_id,
                                     &chht_key, &chht_info) == FALSE) {
        /* Check to see any version mismatch and correct 
         * auxID accordingly. 
         * TODO: Get the actual version from dev and check
         */
        chann_id.aux_id = dp_id;
        if (g_hash_table_lookup_extended(cc_of_global.ofchannel_htbl,
                                         (gconstpointer)&chann_id,
                                         &chht_key, &chht_info) == FALSE) {
            CC_LOG_ERROR("%s(%d): channel %d/%d not found", __FUNCTION__,
                         __LINE__, (int)chann_id.dp_id,
                         (int)chann_id.aux_id);
            g_mutex_unlock(&cc_of_global.ofchannel_htbl_lock);
            return CC_OF_EINVAL;
        }
    }
    chann_info = (cc_ofchannel_info_t *)chht_info;
    send_rwsock = chann_info->rw_sockfd;

    find_thrmgr_rwsocket(send_rwsock, &tmgr);

    if (tmgr == NULL) {
        CC_LOG_ERROR("%s(%d): socket %d is invalid",
                     __FUNCTION__, __LINE__, send_rwsock);
        g_mutex_unlock(&cc_of_global.ofchannel_htbl_lock);
        return CC_OF_EINVAL;
    }
    
    msg_p->hdr.msg_size = msg_len + sizeof(adpoll_send_msg_hdr_t);
    msg_p->hdr.fd = send_rwsock;
    g_memmove(msg_p->data, of_msg, msg_len);

    write(adp_thr_mgr_get_data_pipe_wr(tmgr),
          msg_p, msg_p->hdr.msg_size);
    g_mutex_unlock(&cc_of_global.ofchannel_htbl_lock); 
    return CC_OF_OK;
}


cc_of_ret
cc_of_set_real_dpid_auxid(uint64_t dummy_dpid, uint8_t dummy_auxid,
                          uint64_t dp_id, uint8_t aux_id) 
{
    cc_of_ret status = CC_OF_OK;
    cc_ofchannel_info_t *ofchann_info_old = NULL;
    cc_ofchannel_info_t ofchann_info_new;
    cc_ofchannel_key_t ofchann_key_old, ofchann_key_new;
    gboolean new_entry;
    gpointer chht_key = NULL, chht_info = NULL;

    ofchann_key_new.dp_id = dp_id;
    ofchann_key_new.aux_id = aux_id;
    ofchann_key_old.dp_id = dummy_dpid;
    ofchann_key_old.aux_id = dummy_auxid;

    g_mutex_lock(&cc_of_global.ofchannel_htbl_lock);
    if (g_hash_table_lookup_extended(cc_of_global.ofchannel_htbl, 
                                     &ofchann_key_old,
                                     &chht_key, &chht_info) == FALSE) {
        CC_LOG_ERROR("%s(%d):, could not find ofchann_info in ofchannel_htbl"
                     "for key dummy_dpid-%lu, dummy_auxid-%u",__FUNCTION__, 
                     __LINE__, dummy_dpid, dummy_auxid);
        g_mutex_unlock(&cc_of_global.ofchannel_htbl_lock);
        return CC_OF_EINVAL;
    }
    
    ofchann_info_old = (cc_ofchannel_info_t *)chht_info;

    memcpy(&ofchann_info_new, ofchann_info_old, sizeof(cc_ofchannel_info_t));
    status = del_ofchann_rwsocket((int)dummy_dpid);
    update_global_htbl_lockfree(OFCHANN, DEL, (gpointer)&ofchann_key_new, 
                       &ofchann_info_new, &new_entry); 
    g_mutex_unlock(&cc_of_global.ofchannel_htbl_lock);
    return status;
}


void
cc_of_debug_toggle(gboolean debug_on)
{
    if (debug_on == TRUE) {
        CC_LOG_ENABLE_DEBUGS();
    } else {
        CC_LOG_DISABLE_DEBUGS();
    }
    cc_of_global.ofdebug_enable = debug_on;
}

void
cc_of_log_toggle(gboolean logging_on)
{
    g_mutex_lock(&cc_of_global.oflog_lock);
    if (cc_of_global.oflog_enable == logging_on)  {
        /* do nothing */
        g_mutex_unlock(&cc_of_global.oflog_lock);
        return;
    }
    if (logging_on == TRUE) {
         cc_of_global.oflog_fd =
            create_logfile(cc_of_global.oflog_file);
    } else {
        fclose(cc_of_global.oflog_fd);
    }
    cc_of_global.oflog_enable = logging_on;
    g_mutex_unlock(&cc_of_global.oflog_lock);
}
    
char *
cc_of_log_read()
{
    char *log_contents = NULL;
    g_mutex_lock(&cc_of_global.oflog_lock);
    if (cc_of_global.oflog_enable) {
        log_contents = read_logfile();
    }
    g_mutex_unlock(&cc_of_global.oflog_lock);
    return log_contents;
}

void
cc_of_log_clear()
{
    char clearlog[176];
    sprintf(clearlog, "cat /dev/null > %s", cc_of_global.oflog_file);
    system(clearlog);
}
