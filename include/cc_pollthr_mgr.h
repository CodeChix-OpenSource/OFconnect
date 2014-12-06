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
** Description:    	Poll thread definitions
** Assumptions:         N/A
** Testing:             N/A
**
** Main Contact:        deepa.dhurka@gmail.com
** Alt. Contact:        organizers@codechix.org
****************************************************************
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

#define G_ERRORCHECK_MUTEXES

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

#ifndef UNUSED
#define UNUSED __attribute__ ((__unused__))
#endif

/* Thread-priate data for Polling Thread */
GPrivate tname_key;

typedef enum adpoll_fd_type_ {
    PIPE,
    SOCKET
} adpoll_fd_type_e;

typedef enum adpoll_fd_action_ {
    ADD_FD,
    DELETE_FD
} adpoll_fd_action_e;

/* Global data for async dynamic poll-thread manager */
typedef struct adpoll_thread_mgr {
    char          tname[MAX_NAME_LEN];
    uint16_t      num_sockets;
    uint16_t      num_pipes;
    uint32_t      max_sockets;
    uint32_t      max_pipes;
    int           *pipes_arr;
    GThread       *thread_p;
    GMutex        *add_del_pipe_cv_mutex;
    GCond         *add_del_pipe_cv_cond;
    GMutex        *adp_thr_init_cv_mutex;
    GCond         *adp_thr_init_cv_cond;
} adpoll_thread_mgr_t;

/* parameter for starting new thread manager */
typedef struct adpoll_pollthr_data_ {
    char tname[MAX_NAME_LEN];
    int max_pollfds;
    int primary_pipe_rd_fd;
    adpoll_thread_mgr_t *mgr;
} adpoll_pollthr_data_t;

typedef struct adpoll_send_msg_htbl_key_ {
    int               fd;
    uint64_t          dp_id;
    uint8_t           aux_id; 
} adpoll_send_msg_htbl_key_t;

typedef struct adpoll_send_msg_htbl_info_ {
//    struct pollfd     *pollfd_entry_p; /*poll struct of the rx fd */
    uint              data_size;
    uint64_t          dp_id;
    uint8_t           aux_id;
    char              data[];
} adpoll_send_msg_htbl_info_t;

typedef struct adpoll_fd_info_ adpoll_fd_info_t;

/* Callback function for FD poll-in and poll-out */
typedef void (*fd_process_func)(char *tname,
                                adpoll_fd_info_t *data_p,
                                adpoll_send_msg_htbl_info_t *send_msg_p);

/* message sent via pipe from thread manager to poll thread */
typedef struct adpoll_thr_msg_ {
    int                fd;    
    adpoll_fd_type_e   fd_type;
    adpoll_fd_action_e fd_action;
    short              poll_events; /* poll flags */
    fd_process_func    pollin_func;
    fd_process_func    pollout_func;
} adpoll_thr_msg_t;


/* thread specific */
typedef struct adpoll_fd_info_ {
    int                fd;
    adpoll_fd_type_e   fd_type;
    fd_process_func    pollin_func;
    fd_process_func    pollout_func;
    struct pollfd      *pollfd_entry_p; /*poll syscall uses this info*/
} adpoll_fd_info_t;

typedef struct adpoll_send_msg_hdr_ {
    uint               msg_size;
    int                fd;
    uint64_t           dp_id;
    uint8_t            aux_id;
} adpoll_send_msg_hdr_t;

typedef struct adpoll_send_msg_ {
    adpoll_send_msg_hdr_t hdr;
    char                  data[];
} adpoll_send_msg_t;



typedef struct pollthr_private_ {
    int           num_pollfds;
    struct pollfd *pollfd_arr;
    GList         *fd_list;
    GHashTable    *send_msg_htbl;
    GMutex	  send_msg_htbl_lock;
    GMutex        *add_del_pipe_cv_mutex;
    GCond         *add_del_pipe_cv_cond;
    GMutex        *adp_thr_init_cv_mutex;
    GCond         *adp_thr_init_cv_cond;
} pollthr_private_t;

adpoll_thread_mgr_t *
adp_thr_mgr_new(char *tname,
                uint32_t max_sockets,
                uint32_t max_pipes);

int adp_thr_mgr_add_del_fd(adpoll_thread_mgr_t *this,
                            adpoll_thr_msg_t    *msg);

//not supported yet
//int adp_thr_mgr_get_num_fds(adpoll_thread_mgr_t *this);

/* return value: write pipe fd */
int adp_thr_mgr_get_pri_pipe_wr(adpoll_thread_mgr_t *this);

/* return value: read pipe fd */
int adp_thr_mgr_get_pri_pipe_rd(adpoll_thread_mgr_t *this);

/* return value: write pipe fd */
int adp_thr_mgr_get_data_pipe_wr(adpoll_thread_mgr_t *this);

/* return value: read pipe fd */
int adp_thr_mgr_get_data_pipe_rd(adpoll_thread_mgr_t *this);

uint32_t adp_thr_mgr_get_num_avail_sockfd(adpoll_thread_mgr_t *this);

void adp_thr_mgr_free(adpoll_thread_mgr_t *this);

#endif
