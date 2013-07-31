/*-----------------------------------------------------------------------------*/
/* Copyright: CodeChix Bay Area Chapter 2013                                   */
/*-----------------------------------------------------------------------------*/
#ifndef CC_OF_UTIL_H
#define CC_OF_UTIL_H

#include "cc_pollthr_mgr.h"

typedef enum htbl_update_ops_ {
    ADD,
    DEL
} htbl_update_ops_e;

typedef enum htbl_type_ {
    OFDEV,
    OFRW,
    OFCHANN
} htbl_type_e;

cc_drv_ret
cc_create_rw_pollthr(adpoll_thread_mgr_t *tmgr,
                     uint32_t max_sockets,
                     uint32_t max_pipes);


uint32_t
cc_get_count_rw_pollthr(void);


cc_drv_ret
cc_find_or_create_rw_pollthr(adpoll_thread_mgr_t *tmgr,
                            uint32_t max_sockets,
                            uint32_t max_pipes);


cc_drv_ret
cc_del_sockfd_rw_pollthr(adpoll_thread_mgr_t *tmgr, int fd);


cc_drv_ret
cc_add_sockfd_rw_pollthr(adpoll_thr_msg_t add_fd_msg);


#endif //CC_OF_UTIL_H
