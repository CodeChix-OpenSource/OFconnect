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
/* ASYNCHRONOUS DYNAMIC POLLTHREAD
   This pollthread object is a core unit of thread management.
   A new thread is created with every object instantiation of
   the ADPoll-Thread. Each thread runs a polling loop which can
   process either socket or pipe fds.
   
   2 pipes are used for administrative purposes:
   1. PRIMARY PIPE - used for managing the polled fds in the loop
   2. DATA PIPE - used for sending asynchronously handing data to
                  the loop for sending out on an FD

   Each FD is associated with its own POLLIN and POLLOUT processing
   functions. These callbacks are registered at the time of
   adding an FD to the polling loop via the manager API.

   The thread internally maintains a hashtable for storing the
   data packet that is awaiting a send on an FD, pending a
   POLLOUT on that FD.
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

/* Thread-private data for Polling Thread */
GPrivate tname_key;

typedef enum adpoll_fd_type_ {
    PIPE,
    SOCKET
} adpoll_fd_type_e;

typedef enum adpoll_fd_action_ {
    ADD_FD,
    DELETE_FD
} adpoll_fd_action_e;

/********************************************************************/
/* Asynchronous Dynamic PollThread Manager Object */
/********************************************************************/
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

/********************************************************************/
/* Data Structures for CallBack function for FD processing*/
/********************************************************************/
typedef struct adpoll_send_msg_htbl_info_ {
//    struct pollfd     *pollfd_entry_p; /*poll struct of the rx fd */
    uint              data_size;
    uint64_t          dp_id;
    uint8_t           aux_id;
    char              data[];
} adpoll_send_msg_htbl_info_t;

typedef struct adpoll_fd_info_ adpoll_fd_info_t;

/* Callback function for FD poll-in and poll-out.
   Every FD is mapped to its specific pollin and pollout
   callback functions in the poll-thread
*/
typedef void (*fd_process_func)(char *tname,
                                adpoll_fd_info_t *data_p,
                                adpoll_send_msg_htbl_info_t *send_msg_p);

/* thread specific */
typedef struct adpoll_fd_info_ {
    int                fd;
    adpoll_fd_type_e   fd_type;
    fd_process_func    pollin_func;
    fd_process_func    pollout_func;
    struct pollfd      *pollfd_entry_p; /*poll syscall uses this info*/
} adpoll_fd_info_t;


/********************************************************************/
/* message sent via PRIMARY PIPE from thread manager to poll thread */
/********************************************************************/
typedef struct adpoll_thr_msg_ {
    int                fd;    
    adpoll_fd_type_e   fd_type;
    adpoll_fd_action_e fd_action;
    short              poll_events; /* poll flags */
    fd_process_func    pollin_func;
    fd_process_func    pollout_func;
} adpoll_thr_msg_t;

/********************************************************************/
/* Data that is sent out on the wire. */
/********************************************************************/
/*   This message is placed into the DATA PIPE by ADPollThread Manager
     The polling thread saves it in its hash table until
     a POLLOUT signal is received on the specific fd
*/
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

/********************************************************************/
/* Function: adp_thr_mgr_new
   1. Creates a PRIMARY PIPE
   2. Initializes cond-vars for pipe access and thread initialization
   3. Spawns a thread that runs adp_thr_mgr_poll_thread_func()
   4. The primary pipe fds and cond-vars are sent to new thread during
      thread creation
   5. Adds a new pipe DATA PIPE via adp_thr_mgr_add_del_fd() API
*/
adpoll_thread_mgr_t *
adp_thr_mgr_new(char *tname,
                uint32_t max_sockets,
                uint32_t max_pipes);

/* Function: adp_thr_mgr_add_del_fd
   1. Processes Pipe or Socket
   2. Processes Add or Delete for either type of FD
   3. Creates a new pipe for Pipe Add and cleans up in Delete
*/
int adp_thr_mgr_add_del_fd(adpoll_thread_mgr_t *this,
                            adpoll_thr_msg_t    *msg);


//int adp_thr_mgr_get_num_fds(adpoll_thread_mgr_t *this); NOT SUPPORTED YET

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

/********************************************************************/
/* ADPOLL-THREAD PRIVATE */
/********************************************************************/
/* parameter for starting new thread manager */
typedef struct adpoll_pollthr_data_ {
    char tname[MAX_NAME_LEN];
    int max_pollfds;
    int primary_pipe_rd_fd;
    adpoll_thread_mgr_t *mgr;
} adpoll_pollthr_data_t;

   

/* Thread-private data
   fd_list: master list of FDs with info that
            the thread manages against each
   pollfd_arr: a polling array used by the system call
   send_msg_htbl: hash table to store the data that is waiting to
                  be sent out; pending poll_out on the specific fd
*/
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

typedef struct adpoll_send_msg_htbl_key_ {
    int               fd;
    uint64_t          dp_id;
    uint8_t           aux_id; 
} adpoll_send_msg_htbl_key_t;

#endif
