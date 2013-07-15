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

#define MAX_NAME_LEN   16

#define PRI_PIPE_RD_FD 0
#define PRI_PIPE_WR_FD 1

#define RD_OFFSET      0
#define WR_OFFSET      1

#define IS_FD_WR(fd) (fd % 2)

static GPrivate tname_key;

typedef enum adpoll_fd_type_ {
    PIPE,
    SOCKET
} adpoll_fd_type_e;

typedef enum adpoll_fd_action_ {
    ADD,
    DELETE
} adpoll_fd_action_e;

typedef void (*fd_process_func)(char *tname,
                                void *data_p);

/* async dynamic poll-thread manager */
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
} adpoll_thread_mgr_t;

/* parameter for starting new thread manager */
typedef struct adpoll_pollthr_data_ {
    char tname[MAX_NAME_LEN];
    int max_pollfds;
    int primary_pipe_rd_fd;
    GMutex  *del_pipe_cv_mutex_p;
    GCond   *del_pipe_cv_cond_p;
    
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

void adp_thr_mgr_get_pri_pipe(adpoll_thread_mgr_t *this,
                              int *main_pipe); /* 2 fds returned */

void adp_thr_mgr_free(adpoll_thread_mgr_t *this);

#endif
