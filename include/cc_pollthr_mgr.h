/*-----------------------------------------------------------------------------*/
/* Copyright: CodeChix Bay Area Chapter 2013                                   */
/*-----------------------------------------------------------------------------*/
/*
 * Utility Library
 * Polling Thread Manager does the following:
 *     spawn one Polling Thread and
 *     create two primary pipes, read and write between Manager and Polling Thr
 *     provide capability to add or delete fds (sockets or pipes)
 *         to the polling loop in Polling Thread
 *     provide cleanup API
 *
 * The Polling Thread Manager utility is flexible in that each FD is associated
 *     with its own set of callback functions for POLLIN and POLLOUT
 *
 * adp_thr_mgr_new() - Initialize the instance of polling thread manager
 *                     Creates a new Polling Thread and 2 primary pipes
 *                     Sets up Polling Thread with poll loop with initially
 *                         just the primary read FD
 *
 * adp_thr_mgr_add_del_fd() - Add/Del socket/pipe FDs to the polling loop
 *                                in polling thread
 *                            Del of primary pipe FD will self-destruct this
 *                                instance of Polling Thread Manager
 *
 * adp_thr_mgr_free() - Cleans up an instance of Polling Thread Manager
 *
 * Sample code
 adpoll_thread_mgr_t *tmgr;    
        
 tmgr = adp_thr_mgr_new("aaroh", 2, 10);

 * Add pipe
 add_fd_msg.fd_type = PIPE;
 add_fd_msg.fd_action = ADD_FD;
 add_fd_msg.poll_events = POLLIN;
 add_fd_msg.pollin_func = NULL;
 add_fd_msg.pollout_func = NULL;
 
 new_pipe_wr_fd1 = adp_thr_mgr_add_del_fd(tmgr, &add_fd_msg);
 if (new_pipe_wr_fd1 != -1) {
 CC_LOG_DEBUG("%s(%d): successful new write pipe: %d",
 __FUNCTION__, __LINE__, new_pipe_wr_fd1);
 }
 new_pipe_wr_fd2 = adp_thr_mgr_add_del_fd(tmgr, &add_fd_msg);
 
 * Delete pipe
 add_fd_msg.fd_type = PIPE;
 add_fd_msg.fd_action = DELETE_FD;
 add_fd_msg.fd = new_pipe_wr_fd1;
 add_fd_msg.pollin_func = NULL;
 add_fd_msg.pollout_func = NULL;
 
 adp_thr_mgr_add_del_fd(tmgr, &add_fd_msg);

 * Delete primary pipe to self destruct
 
 add_fd_msg.fd_type = PIPE;
 add_fd_msg.fd_action = DELETE_FD;
 add_fd_msg.fd = PRI_PIPE_WR_FD;
 add_fd_msg.pollin_func = NULL;
 add_fd_msg.pollout_func = NULL;
 adp_thr_mgr_add_del_fd(tmgr, &add_fd_msg);
 adp_thr_mgr_free(tmgr);
*/
 
#ifndef CC_POLLTHR_MGR_H
#define CC_POLLTHR_MGR_H

#include <glib.h>
#include <glib/gprintf.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

/* length of name string for Polling Thread */
#define MAX_NAME_LEN   16

/* Index of primary pipes in pipes_arr */
#define PRI_PIPE_RD_FD 0
#define PRI_PIPE_WR_FD 1

/* Index of send-data pipes in pipes_arr */
#define DATA_PIPE_RD_FD 2
#define DATA_PIPE_WR_FD 3

/* Index offsets for read and write pipes */
#define RD_OFFSET      0
#define WR_OFFSET      1

#define IS_FD_WR(fd) (fd % 2)

/* Thread-priate data for Polling Thread */
static GPrivate tname_key;

typedef enum adpoll_fd_type_ {
    PIPE,
    SOCKET
} adpoll_fd_type_e;

typedef enum adpoll_fd_action_ {
    ADD_FD,
    DELETE_FD
} adpoll_fd_action_e;

/* Callback function for FD poll-in and poll-out */
typedef void (*fd_process_func)(char *tname,
                                void *data_p);

/* Global data for async dynamic poll-thread manager */
typedef struct adpoll_thread_mgr {
    char          tname[MAX_NAME_LEN];
    uint16_t      num_sockets;
    uint16_t      num_pipes;
    uint32_t      max_sockets;
    uint32_t      max_pipes;
    int           *pipes_arr;
    GThread       *thread_p;
    GMutex        del_pipe_cv_mutex;
    GCond         del_pipe_cv_cond;
    GMutex        adp_thr_init_cv_mutex;
    GCond         adp_thr_init_cv_cond;
} adpoll_thread_mgr_t;

/* parameter for starting new thread manager */
typedef struct adpoll_pollthr_data_ {
    char tname[MAX_NAME_LEN];
    int max_pollfds;
    int primary_pipe_rd_fd;
    GMutex  *del_pipe_cv_mutex_p;
    GCond   *del_pipe_cv_cond_p;
    GMutex  *adp_thr_init_cv_mutex_p;
    GCond   *adp_thr_init_cv_cond_p;
} adpoll_pollthr_data_t;

/* message sent via pipe from thread manager to poll thread */
typedef struct adpoll_thr_msg_ {
    int                fd;    
    adpoll_fd_type_e   fd_type;
    adpoll_fd_action_e fd_action;
    short              poll_events; /* poll flags */
    fd_process_func    pollin_func;
    void               *pollin_user_data;
    fd_process_func    pollout_func;
    void               *pollout_user_data;
} adpoll_thr_msg_t;

/* thread specific */
typedef struct adpoll_fd_info_ {
    int                fd;
    adpoll_fd_type_e   fd_type;
    fd_process_func    pollin_func;
    void               *pollin_user_data;
    fd_process_func    pollout_func;
    void               *pollout_user_data;
    struct pollfd      *pollfd_entry_p; /*poll syscall uses this info*/
} adpoll_fd_info_t;

typedef struct pollthr_private_ {
    int           num_pollfds;
    struct pollfd *pollfd_arr;
    GList         *fd_list;
    GMutex        *del_pipe_cv_mutex_p;
    GCond         *del_pipe_cv_cond_p;
} pollthr_private_t;

adpoll_thread_mgr_t *
adp_thr_mgr_new(char *tname,
                uint32_t max_sockets,
                uint32_t max_pipes);

int adp_thr_mgr_add_del_fd(adpoll_thread_mgr_t *this,
                            adpoll_thr_msg_t    *msg);

int adp_thr_mgr_get_num_fds(adpoll_thread_mgr_t *this);

/* return value: write pipe fd */
int adp_thr_mgr_get_pri_pipe_wr(adpoll_thread_mgr_t *this);

/* return value: write pipe fd */
int adp_thr_mgr_get_data_pipe_wr(adpoll_thread_mgr_t *this);

uint32_t adp_thr_mgr_get_num_avail_sockfd(adpoll_thread_mgr_t *this);

void adp_thr_mgr_free(adpoll_thread_mgr_t *this);

#endif
