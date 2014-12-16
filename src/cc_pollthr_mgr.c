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

#include "cc_pollthr_mgr.h"
#include "cc_log.h"
#include "cc_of_global.h"
#include "errno.h"

#define FD_LIST_COUNT_LOG "%s(%d)[%s]: fd_list has %d entries "
#define POLLFD_COUNT_LOG "%s(%d)[%s]: num pollfds is %d "
#define PIPEFD_CREATE_LOG "%s(%d)[%s]: pipe fds created [rd][wr]: [%d][%d] "

/* Forward Declarations */
void
adp_thr_mgr_poll_thread_func(adpoll_pollthr_data_t *pollthr_data_p);

static void
pollthr_data_pipe_process_func(char *tname,
                               adpoll_fd_info_t *data_p,
                               adpoll_send_msg_htbl_info_t *unused_data UNUSED);                               

/* Utility functions */
gint
fdinfo_compare_fd(gconstpointer list_elem, gconstpointer compare_fd)
{
    adpoll_fd_info_t *list_elem_p = (adpoll_fd_info_t *)list_elem;
    int fd = *(int *)compare_fd;
    
    if (list_elem_p->fd < fd) {
        return -1;
    } else if (list_elem_p->fd > fd) {
        return 1;
    } else {
        return 0;
    }
    return 0;
}

/* Instantiation of an ADPoll-Thread Manager*/
adpoll_thread_mgr_t *
adp_thr_mgr_new(char *tname,
                uint32_t max_sockets,
                uint32_t max_pipes)
{
    int i = 0;
    adpoll_thread_mgr_t *this = NULL;
    adpoll_pollthr_data_t *thread_user_data;
    int tname_len;
    adpoll_thr_msg_t add_datapipe_msg;

    /* Allocate for the ADPoll-Thread Manager object
       This is stored in a global list by the calling function
    */
    this = (adpoll_thread_mgr_t *)malloc(sizeof(adpoll_thread_mgr_t));

    /* Initialize the cond-vars for pipe access and thread init */
    this->add_del_pipe_cv_mutex = g_mutex_new();
    this->add_del_pipe_cv_cond = g_cond_new();
    this->adp_thr_init_cv_mutex = g_mutex_new();
    this->adp_thr_init_cv_cond = g_cond_new();

    while ((tname[i] != 0) && (i < (MAX_NAME_LEN - 2))) {
        this->tname[i] = tname[i];
        i++;
    }
    this->tname[i] = 0; /* NULL terminate */
    tname_len = i + 1;
    
    this->max_sockets = max_sockets;

    /* 2fds per pipe; 4fds additional for internal use */    
    this->max_pipes = max_pipes*2 + 4;
    this->num_pipes = 0;
    this->num_sockets = 0;

    this->pipes_arr = (int *)malloc(sizeof(int) * 2 * this->max_pipes);

    g_mutex_init(this->add_del_pipe_cv_mutex);
    g_cond_init(this->add_del_pipe_cv_cond);
    g_mutex_init(this->adp_thr_init_cv_mutex);
    g_cond_init(this->adp_thr_init_cv_cond);

    /* create PRIMARY pipe - read is in location 0 and write in 1 */
    if (pipe(this->pipes_arr) == -1) {
        CC_LOG_FATAL("%s(%d): pipe creation failed",__FUNCTION__,
                     __LINE__);
    }
    this->num_pipes += 2; /* pipe creates 2 fds */

    CC_LOG_DEBUG(PIPEFD_CREATE_LOG "PRIMARY",
                 __FUNCTION__, __LINE__, tname,
                 this->pipes_arr[PRI_PIPE_RD_FD],
                 this->pipes_arr[PRI_PIPE_WR_FD]);

    /* Setup information to be sent to the thread during creation */
    thread_user_data = (adpoll_pollthr_data_t *)
        malloc(sizeof(adpoll_pollthr_data_t));

    strncpy(thread_user_data->tname, this->tname, tname_len);
    thread_user_data->max_pollfds = max_sockets + this->max_pipes;
    thread_user_data->primary_pipe_rd_fd = this->pipes_arr[PRI_PIPE_RD_FD];

    thread_user_data->mgr = this;
    CC_LOG_DEBUG("%s(%d): tname is %s", __FUNCTION__, __LINE__,
                 thread_user_data->tname);
    this->thread_p = g_thread_new(this->tname,
                            (GThreadFunc) adp_thr_mgr_poll_thread_func,
                            thread_user_data);

    /* synchronize with thr_mgr_poll_thread_func
       Wait till thread is created*/
    g_mutex_lock(this->adp_thr_init_cv_mutex);
    
    g_cond_wait(this->adp_thr_init_cv_cond,
                this->adp_thr_init_cv_mutex);
    g_mutex_unlock(this->adp_thr_init_cv_mutex);


    /* Add a DATA PIPE via the manager's own API */
    add_datapipe_msg.fd_type = PIPE;
    add_datapipe_msg.fd_action = ADD_FD;
    add_datapipe_msg.poll_events = POLLIN;
    /* pollin_func pulls data from the data pipe
       and stores in a hash table based on rwsocket
    */
    add_datapipe_msg.pollin_func = &pollthr_data_pipe_process_func;
    add_datapipe_msg.pollout_func = NULL;

    adp_thr_mgr_add_del_fd(this, &add_datapipe_msg);
    CC_LOG_DEBUG("%s(%d): new pipe added for data %d",
                 __FUNCTION__, __LINE__,
                 this->pipes_arr[DATA_PIPE_WR_FD]);
    
    return this;
}

/* NOTE: application needs to clear the 'this' pointer */
void adp_thr_mgr_free(adpoll_thread_mgr_t *this)
{
    int i;
    adpoll_thr_msg_t destruct_msg;

    destruct_msg.fd = this->pipes_arr[PRI_PIPE_RD_FD];
    destruct_msg.fd_type = PIPE;
    destruct_msg.fd_action = DELETE_FD;
    destruct_msg.poll_events = 0;
    destruct_msg.pollin_func = NULL;
    destruct_msg.pollout_func = NULL;

    adp_thr_mgr_add_del_fd(this, &destruct_msg);
    
    /* wait for join */
    g_thread_join (this->thread_p);

    for(i=0; i < this->num_pipes; i++) {
        close(this->pipes_arr[i]);
    }
    
    free(this->pipes_arr);

    g_cond_clear(this->adp_thr_init_cv_cond);
    g_mutex_clear(this->adp_thr_init_cv_mutex);
    
    g_cond_clear(this->add_del_pipe_cv_cond);
    g_mutex_clear(this->add_del_pipe_cv_mutex);    
}

/* Function: adp_thr_mgr_add_del_fd
 * API to create or remove an fd
 * The fd could be either a pipe or a network socket
 * return value: ADD_FD - the newly created wr pipe is returned
 *             : DELETE_FD - returns -1.
 */
int
adp_thr_mgr_add_del_fd(adpoll_thread_mgr_t *this,
                       adpoll_thr_msg_t    *msg)
{
    int i, j, retval = -1;
    gboolean self_destruct = FALSE;
    int del_rd_fd_index;
    uint num_pipes;
    num_pipes = this->num_pipes;

    if (msg->fd_type == PIPE) {
        if (msg->fd_action == ADD_FD) {
            CC_LOG_DEBUG("%s(%d)[%s]: pipe ADD", __FUNCTION__,
                         __LINE__, this->tname);
            
            CC_LOG_DEBUG("%s(%d)[%s]: numpipes is %d", __FUNCTION__,
                         __LINE__, this->tname, num_pipes);

            if (num_pipes >= this->max_pipes) {
                CC_LOG_ERROR("%s(%d)[%s]: unable to add more pipes - max out",
                             __FUNCTION__, __LINE__, this->tname);
                return retval;
            }
            int new_rd_fd_index = num_pipes + RD_OFFSET;

            if (pipe(&this->pipes_arr[num_pipes]) == -1) {
                CC_LOG_FATAL("%s(%d)[%s]: pipe creation failed error %s",
                             __FUNCTION__, __LINE__, this->tname,
                             g_strerror(errno));
                return retval;
            }
            CC_LOG_DEBUG(PIPEFD_CREATE_LOG "ADD-ON",
                         __FUNCTION__, __LINE__, this->tname,
                         this->pipes_arr[num_pipes],
                         this->pipes_arr[num_pipes + 1]);

            num_pipes += 2;
            this->num_pipes = num_pipes; /* pipe creates 2 fds */

            msg->fd = this->pipes_arr[new_rd_fd_index];

            g_mutex_lock(this->add_del_pipe_cv_mutex);
            write(this->pipes_arr[PRI_PIPE_WR_FD],
                  msg, sizeof(adpoll_thr_msg_t));

            retval = this->pipes_arr[num_pipes - 2 + WR_OFFSET];
            
        } else if (msg->fd_action == DELETE_FD) {
            /* find this fd in pipes_arr */

            
            CC_LOG_DEBUG("%s(%d)[%s]:pipe %d DELETE",
                         __FUNCTION__, __LINE__, this->tname,
                         msg->fd);
            if ((msg->fd == this->pipes_arr[PRI_PIPE_RD_FD]) ||
                (msg->fd == this->pipes_arr[PRI_PIPE_WR_FD])) {

                /* Update the message to send to poll thr */
                msg->fd = this->pipes_arr[PRI_PIPE_RD_FD];
                
                CC_LOG_DEBUG("%s(%d)[%s]: sending fd DEL to poll thr on fd %d",
                             __FUNCTION__, __LINE__, this->tname,
                             this->pipes_arr[PRI_PIPE_WR_FD]);

                g_mutex_lock(this->add_del_pipe_cv_mutex);
                write(this->pipes_arr[PRI_PIPE_WR_FD],
                          msg, sizeof(adpoll_thr_msg_t));

                self_destruct = TRUE;

            } else if ((msg->fd == this->pipes_arr[DATA_PIPE_RD_FD]) ||
                       (msg->fd == this->pipes_arr[DATA_PIPE_WR_FD])) {
                    
                /* Update the message to send to poll thr */
                msg->fd = this->pipes_arr[DATA_PIPE_RD_FD];
                
                CC_LOG_DEBUG("%s(%d)[%s]: sending fd DEL to poll thr on fd %d",
                             __FUNCTION__, __LINE__, this->tname,
                             this->pipes_arr[DATA_PIPE_WR_FD]);

                g_mutex_lock((this->add_del_pipe_cv_mutex));
                write(this->pipes_arr[PRI_PIPE_WR_FD],
                          msg, sizeof(adpoll_thr_msg_t));

                self_destruct = TRUE;                
            } else {
                
                for (i = 0; i < this->num_pipes; i++) {
                    if (this->pipes_arr[i] == msg->fd) {
                        del_rd_fd_index = i;
                        if (IS_FD_WR(del_rd_fd_index)) {
                            del_rd_fd_index -= WR_OFFSET;
                        } else {
                            del_rd_fd_index -= RD_OFFSET;
                        }
                        
                        CC_LOG_DEBUG("%s(%d)[%s]: found the fd %d in pipes_arr",
                                     __FUNCTION__, __LINE__, this->tname,
                                     this->pipes_arr[i]);
                        
                        /* Update the message to send to poll thr */
                        msg->fd = this->pipes_arr[del_rd_fd_index];
                        
                        /* process error return */
                        CC_LOG_DEBUG("%s(%d)[%s]: sending fd DEL to poll thr on fd %d",
                                     __FUNCTION__, __LINE__, this->tname,
                                     this->pipes_arr[PRI_PIPE_WR_FD]);
                        
                        g_mutex_lock((this->add_del_pipe_cv_mutex));
                        write(this->pipes_arr[PRI_PIPE_WR_FD],
                              msg, sizeof(adpoll_thr_msg_t));
                        
                        break;
                    }
                }
            }
        }
    } else if (msg->fd_type == SOCKET) {
        if (msg->fd_action == ADD_FD) {
            CC_LOG_DEBUG("%s(%d)[%s]: socket %d ADD",
                         __FUNCTION__, __LINE__, this->tname, msg->fd);

            if (this->num_sockets == this->max_sockets) {
                CC_LOG_ERROR("%s(%d)[%s]: unable to add more sockets - "
                             "max out", __FUNCTION__, __LINE__, this->tname);
                return retval;
            }
            this->num_sockets += 1;
            
            g_mutex_lock((this->add_del_pipe_cv_mutex));
            
            write(this->pipes_arr[PRI_PIPE_WR_FD],
                  msg, sizeof(adpoll_thr_msg_t));

            retval = msg->fd;

        } else if (msg->fd_action == DELETE_FD) {
            CC_LOG_DEBUG("%s(%d)[%s]: socket %d DELETE",
                         __FUNCTION__, __LINE__, this->tname, msg->fd);

            g_mutex_lock((this->add_del_pipe_cv_mutex));            
            write(this->pipes_arr[PRI_PIPE_WR_FD],
                  msg, sizeof(adpoll_thr_msg_t));
        }
    }

    g_cond_wait((this->add_del_pipe_cv_cond),
                (this->add_del_pipe_cv_mutex));
    g_mutex_unlock((this->add_del_pipe_cv_mutex));

    if (self_destruct) {
        /* DO NOT CALL adp_thr_mgr_free here -
         * causes deadlock as it triggers antoher self destruct
         */
        return retval;
    }

    if (msg->fd_action == DELETE_FD) {
        if (msg->fd_type == PIPE) {
            close(this->pipes_arr[del_rd_fd_index]);
            close(this->pipes_arr[del_rd_fd_index + WR_OFFSET]);
            
            for (j = del_rd_fd_index + 2; j < this->num_pipes; j++) {
                this->pipes_arr[j-2] = this->pipes_arr[j];
            }
            
            this->num_pipes -= 2;
        } else {
            (this->num_sockets)--;
        }
    }
    return retval;
}


void
fd_entry_free(adpoll_fd_info_t *data)
{
    free(data);
}

/* Function: poll_fd_process

   This function is called by the polling loop in
   adp_thr_mgr_poll_thread_func when poll() returns
   a valid event.
   
   poll_fd_process iterates through the entire fd_list
   and processes the specific event on the correct fd.

   POLLIN: call the registered callback for processing
   POLLOUT: retrieve the relevant data from hash table
   and send out on fd
*/
static void
poll_fd_process(adpoll_fd_info_t *data_p,
                char *tname)
{
    pollthr_private_t *thr_pvt_p = NULL;
    int send_msg_key_fd;
    adpoll_send_msg_htbl_info_t *send_msg_info;    
    
    thr_pvt_p = g_private_get(&tname_key);

    if ((data_p->pollfd_entry_p) &&
        ((data_p->pollfd_entry_p->revents & POLLIN) &
         (data_p->pollfd_entry_p->events & POLLIN)))
    {
        CC_LOG_DEBUG("%s(%d)[%s]: POLLIN on fd %d",
                     __FUNCTION__, __LINE__, tname,
                     data_p->pollfd_entry_p->fd);
        if (data_p->pollin_func) {
            data_p->pollin_func(tname, data_p, NULL);
        }
    }
    if ((data_p->pollfd_entry_p) &&
        ((data_p->pollfd_entry_p->revents & POLLOUT) &
         (data_p->pollfd_entry_p->events & POLLOUT)))
    {
        CC_LOG_DEBUG("%s(%d)[%s]: POLLOUT on fd %d",
                     __FUNCTION__, __LINE__, tname,
                     data_p->pollfd_entry_p->fd);
        
        /* lookup hash table entry for this socket fd*/
        send_msg_key_fd = data_p->pollfd_entry_p->fd;

        g_mutex_lock(&thr_pvt_p->send_msg_htbl_lock);
        
        CC_LOG_DEBUG("%s(%d)[%s]: fd key looked up in hash table: %d",
                     __FUNCTION__, __LINE__, tname,
                     send_msg_key_fd);

        if(!g_hash_table_contains(thr_pvt_p->send_msg_htbl,
                                  GINT_TO_POINTER (send_msg_key_fd))) {
            CC_LOG_ERROR("%s(%d)[%s]: hash table does not have fd %d; "
                         "hash table size is %d", __FUNCTION__, __LINE__,
                         tname, send_msg_key_fd,
                         g_hash_table_size(thr_pvt_p->send_msg_htbl));
        }
        
        send_msg_info = g_hash_table_lookup(
            thr_pvt_p->send_msg_htbl,
            GINT_TO_POINTER (send_msg_key_fd));

        g_assert(send_msg_info != NULL);
        g_private_replace(&tname_key,
                          (gpointer)thr_pvt_p);
        
        if (data_p->pollout_func) {
            data_p->pollout_func(tname, data_p, send_msg_info);
        } else {
            CC_LOG_ERROR("%s(%d)[%s]: No pollout function defined",
                         __FUNCTION__, __LINE__, tname);
        }
        
        thr_pvt_p = g_private_get(&tname_key);        
        
        if (!g_hash_table_remove(thr_pvt_p->send_msg_htbl,
                                 GINT_TO_POINTER (send_msg_key_fd))) {
            CC_LOG_ERROR("%s(%d)[%s]: htbl delete failed for fd %d",
                         __FUNCTION__, __LINE__, tname, send_msg_key_fd);
        }

        /* if this is the last of the messages for this fd,
         *  reset pollout flag */
        if(!g_hash_table_contains(thr_pvt_p->send_msg_htbl,
                                  GINT_TO_POINTER (send_msg_key_fd))) {
            
            CC_LOG_DEBUG("%s(%d)[%s]: Resetting POLLOUT flag for fd %d",
                         __FUNCTION__, __LINE__, tname, send_msg_key_fd);
            data_p->pollfd_entry_p->events &= ~POLLOUT;
        }
        g_mutex_unlock(&thr_pvt_p->send_msg_htbl_lock);
    }
    
    g_private_replace(&tname_key,
                      (gpointer)thr_pvt_p);
}

static void
print_fd_list(adpoll_fd_info_t *data_p)
{
    CC_LOG_DEBUG("fd: %d\tfd_type: %d\tpollin_func: %p\tpollout_func: %p \tevents: %x",
                data_p->fd, data_p->fd_type, data_p->pollin_func,
                data_p->pollout_func, data_p->pollfd_entry_p->events);
}



/* Function: pollthr_pri_pipe_process_func
 * Callback function to process a pipe read
 * This function is of type fd_process_func
 */
static void
pollthr_pri_pipe_process_func(char *tname,
                              adpoll_fd_info_t *data_p,
                              adpoll_send_msg_htbl_info_t *unused_data UNUSED)
{
    adpoll_thr_msg_t msg;
    adpoll_fd_info_t *fd_entry_p; /* append this entry to fd_list */
    int i;
    struct pollfd *pollfd_entry_p;
    GList *traverse = NULL;
    
    pollthr_private_t *thr_pvt_p = NULL;
    
    thr_pvt_p = g_private_get(&tname_key);

    read(data_p->fd, &msg, sizeof(adpoll_thr_msg_t));
        
    CC_LOG_DEBUG("%s(%d)[%s]: message received: fd type: %d,"
                " fd %d, fd action %d, poll events %d,"
                " in func: %p, out func: %p",
                __FUNCTION__, __LINE__, tname,
                msg.fd_type, msg.fd, msg.fd_action,
                msg.poll_events, msg.pollin_func,
                msg.pollout_func);

    switch (msg.fd_action) {
      case ADD_FD:
      {

          CC_LOG_DEBUG("%s(%d)[%s]: fd ADD %d of type %d of action %d",
                       __FUNCTION__, __LINE__, tname, msg.fd,
                       msg.fd_type, msg.fd_action);

          /* check for existing entry */
          g_assert(g_list_find_custom(
                       thr_pvt_p->fd_list,
                       &msg.fd,
                       fdinfo_compare_fd) == NULL);
          
          fd_entry_p = (adpoll_fd_info_t *)malloc(sizeof(adpoll_fd_info_t));
          fd_entry_p->fd = msg.fd;
          fd_entry_p->fd_type = msg.fd_type;
            
          fd_entry_p->pollin_func = msg.pollin_func;
        
          fd_entry_p->pollout_func = msg.pollout_func;

          /* access and modify the polling thread's pollfd array */
          /* add a corresponding pollfd entry */
          pollfd_entry_p = &(thr_pvt_p->pollfd_arr[thr_pvt_p->num_pollfds]);

          thr_pvt_p->num_pollfds += 1;

          CC_LOG_DEBUG(POLLFD_COUNT_LOG "after ADD_FD",
                       __FUNCTION__, __LINE__, tname,
                       thr_pvt_p->num_pollfds);
          
          pollfd_entry_p->fd = msg.fd;
          pollfd_entry_p->events = msg.poll_events;

          /* remove POLLOUT until message is buffered to send out */
          pollfd_entry_p->events &= ~(POLLOUT);

          fd_entry_p->pollfd_entry_p = pollfd_entry_p;
          
          thr_pvt_p->fd_list = g_list_append(thr_pvt_p->fd_list, fd_entry_p);

          if(cc_of_global.ofut_enable) {
              CC_LOG_DEBUG(FD_LIST_COUNT_LOG "ADD_FD", __FUNCTION__, __LINE__,
                           tname, g_list_length(thr_pvt_p->fd_list));
          }
          
      }
      break;      
      case DELETE_FD:
      {
          gboolean found = FALSE;
          int del_index;
          CC_LOG_DEBUG("%s(%d)[%s]: fd DELETE", __FUNCTION__, __LINE__,
                       tname);
          
          if ((msg.fd == thr_pvt_p->pollfd_arr[0].fd) ||
              (msg.fd == thr_pvt_p->pollfd_arr[1].fd)) {
              CC_LOG_DEBUG("%s(%d)[%s]: Received DEL on primary pipe FD "
                           "or data pipe FD - SELF DESTRUCT",
                           __FUNCTION__, __LINE__, tname);
              
              thr_pvt_p->num_pollfds = 0;
              
              g_private_replace(&tname_key,
                                (gpointer)thr_pvt_p);
              return;
          } else {
              
              /* find and delete the pollfd entry */              
              for (i = 1; i < thr_pvt_p->num_pollfds; i++) {
                  if (msg.fd == thr_pvt_p->pollfd_arr[i].fd) {
                      del_index = i;
                      found = TRUE;
                      break;
                  }
              }
              if (found == TRUE) {
                  CC_LOG_DEBUG("%s(%d)[%s]: found fd in pollfd_arr",
                               __FUNCTION__, __LINE__, tname);
                  
                  for (i = del_index + 1 ; i< thr_pvt_p->num_pollfds; i++) {
                      thr_pvt_p->pollfd_arr[i-1].fd =
                          thr_pvt_p->pollfd_arr[i].fd;
                      
                      thr_pvt_p->pollfd_arr[i-1].events =
                          thr_pvt_p->pollfd_arr[i].events;
                      
                      thr_pvt_p->pollfd_arr[i-1].revents =
                          thr_pvt_p->pollfd_arr[i].revents;
                  }
                  thr_pvt_p->num_pollfds--;
                  
                  CC_LOG_DEBUG(POLLFD_COUNT_LOG "after DELETE_FD",
                               __FUNCTION__, __LINE__, tname,
                               thr_pvt_p->num_pollfds);
                  
                  /* find and delete the list entry for this pipe fd */
                  traverse = g_list_first(thr_pvt_p->fd_list);
                  fd_entry_p = NULL;
                  
                  while (traverse != NULL) {
                      if (((adpoll_fd_info_t *)(traverse->data))->fd == msg.fd) {
                          fd_entry_p = (adpoll_fd_info_t *)(traverse->data);
                          break;
                      }
                      traverse = g_list_next(traverse);
                  }
                  
                  if (fd_entry_p) {
                      thr_pvt_p->fd_list = g_list_remove(thr_pvt_p->fd_list,
                                                         (gconstpointer) fd_entry_p);
                  } else {
                      
                      CC_LOG_ERROR("%s(%d)[%s]: inconsistent database "
                                   "- pollfd found but fd entry not in list",
                                   __FUNCTION__, __LINE__, tname);
                  }
                  if(cc_of_global.ofut_enable) {
                      CC_LOG_DEBUG(FD_LIST_COUNT_LOG "DELETE_FD", __FUNCTION__, __LINE__,
                                   tname, g_list_length(thr_pvt_p->fd_list));
                  }
              } else {
                  CC_LOG_ERROR("%s(%d)[%s]: NOT found fd in pollfd_arr",
                               __FUNCTION__, __LINE__, tname);
              }
          }
      }
      break;
      default:
        CC_LOG_FATAL("%s(%d)[%s]: neither ADD_FD nor DELETE_FD",
                     __FUNCTION__, __LINE__, tname);
    }

    g_private_replace(&tname_key,
                      (gpointer)thr_pvt_p);
    
    g_mutex_lock((thr_pvt_p->add_del_pipe_cv_mutex));
    g_cond_signal((thr_pvt_p->add_del_pipe_cv_cond));
    g_mutex_unlock((thr_pvt_p->add_del_pipe_cv_mutex));
}

static void
pollthr_data_pipe_process_func(char *tname,
                               adpoll_fd_info_t *data_p,
                               adpoll_send_msg_htbl_info_t *unused_data UNUSED)
{
    char msg_buf[SEND_MSG_BUF_SIZE];
    adpoll_send_msg_t *msg_p;
    struct pollfd *pollfd_entry_p;
    int send_msg_key_fd;
    adpoll_send_msg_htbl_info_t *send_msg_info;
    GList *rdfd_info_list = NULL;
    adpoll_fd_info_t *rdfd_info;
    int data_size;

    pollthr_private_t *thr_pvt_p = NULL;    
    thr_pvt_p = g_private_get(&tname_key);
    
    read(data_p->fd, msg_buf, SEND_MSG_BUF_SIZE);
    msg_p = (adpoll_send_msg_t *)msg_buf;
    
    CC_LOG_DEBUG("%s(%d)[%s]: message received: size: %u, fd : %d",
                 __FUNCTION__, __LINE__, tname,
                 msg_p->hdr.msg_size, msg_p->hdr.fd);

    send_msg_key_fd = msg_p->hdr.fd;
    data_size = msg_p->hdr.msg_size - sizeof(adpoll_send_msg_hdr_t);

    send_msg_info = malloc(sizeof(adpoll_send_msg_htbl_info_t) + data_size);
    send_msg_info->data_size = data_size;
    g_memmove(send_msg_info->data, msg_p->data, data_size);

    /* add message to htbl */
    g_mutex_lock(&thr_pvt_p->send_msg_htbl_lock);

    g_hash_table_insert(thr_pvt_p->send_msg_htbl,
                        GINT_TO_POINTER(send_msg_key_fd),
                        (gpointer)send_msg_info);
    
    /* update POLLOUT flag on pollfd entry so it can be sent out */
    /* find the pollfd_entry_p of the send_msg_key->fd descriptor */
    /* data_p->pollfd_entry_p is that of the data pipe!! */
    rdfd_info_list =  g_list_find_custom(
        thr_pvt_p->fd_list,
        &send_msg_key_fd,
        fdinfo_compare_fd);

    g_assert(rdfd_info_list != NULL);
    
    rdfd_info = g_list_nth_data (rdfd_info_list, 0);

    g_assert(rdfd_info != NULL);
    
    pollfd_entry_p = rdfd_info->pollfd_entry_p;

    g_assert(pollfd_entry_p != NULL);
    
    pollfd_entry_p->events |= POLLOUT;

    g_mutex_unlock(&thr_pvt_p->send_msg_htbl_lock);
    
    g_private_replace(&tname_key,
                      (gpointer)thr_pvt_p);
}

void func_destroy_key(gpointer data UNUSED)
{
    /* noop since we are using GINT_TO_POINTER */
    return;
}

void func_destroy_val(gpointer data)
{
    /* freeing of 'data' is sufficient to free the entire node */
    free(data);
}


/*
 * Function: adp_thr_mgr_poll_thread_func
 * adp_thr_mgr_new() creates a poll thread that runs this function
 * This function polls on pipe and socket fds in a loop
 * The first pipe is setup permanently with the parent thread
 * Parent thread sends new socket and pipe fds to poll on via the
 *   permanent pipe.
 * The permanent pipe is deleted only when the poll thread is torn down
 */
void
adp_thr_mgr_poll_thread_func(adpoll_pollthr_data_t *pollthr_data_p)
{
    int rv, i;
    adpoll_fd_info_t *fd_entry_p;
    pollthr_private_t *thr_pvt_p;
    char pollthr_name[MAX_NAME_LEN];
    GList *thr_pvt_fd_list = NULL;

    if (pollthr_data_p == NULL) {
        CC_LOG_FATAL("%s(%d): received NULL user data", __FUNCTION__, __LINE__);
    }
    
    strncpy(pollthr_name, pollthr_data_p->tname, MAX_NAME_LEN);
    
    CC_LOG_DEBUG("%s(%d)[%s]: thread started with max pollfds %d"
                 " and primary pipe rd fd %d", __FUNCTION__,
                 __LINE__, pollthr_name,
                 pollthr_data_p->max_pollfds,
                 pollthr_data_p->primary_pipe_rd_fd);
    
    /* Initialize thread private data */
    thr_pvt_p = (pollthr_private_t *)malloc(sizeof(pollthr_private_t));
    thr_pvt_p->pollfd_arr = (struct pollfd *)malloc(sizeof(struct pollfd) *
                                                    pollthr_data_p->max_pollfds);
    thr_pvt_p->fd_list = NULL;

    thr_pvt_p->add_del_pipe_cv_mutex = pollthr_data_p->mgr->add_del_pipe_cv_mutex;
    thr_pvt_p->add_del_pipe_cv_cond = pollthr_data_p->mgr->add_del_pipe_cv_cond;
    thr_pvt_p->adp_thr_init_cv_mutex = pollthr_data_p->mgr->adp_thr_init_cv_mutex;
    thr_pvt_p->adp_thr_init_cv_cond = pollthr_data_p->mgr->adp_thr_init_cv_cond;


    g_mutex_init(&thr_pvt_p->send_msg_htbl_lock);
    thr_pvt_p->send_msg_htbl = g_hash_table_new_full(g_direct_hash,
                                                     g_direct_equal,
                                                     func_destroy_key,
                                                     func_destroy_val);

    /* Initialize the first fd entry in fd_list */
    fd_entry_p = (adpoll_fd_info_t *)malloc(sizeof(adpoll_fd_info_t));
    fd_entry_p->fd = pollthr_data_p->primary_pipe_rd_fd;
    fd_entry_p->fd_type = PIPE;
    fd_entry_p->pollin_func = &pollthr_pri_pipe_process_func;
    fd_entry_p->pollout_func = NULL;

    /* setup poll fd for primary pipe*/
    thr_pvt_p->pollfd_arr[0].fd = fd_entry_p->fd;
    thr_pvt_p->pollfd_arr[0].events = POLLIN;
    thr_pvt_p->num_pollfds = 1;

    fd_entry_p->pollfd_entry_p = &(thr_pvt_p->pollfd_arr[0]);
    thr_pvt_p->fd_list = g_list_append(thr_pvt_p->fd_list, fd_entry_p);

    if(cc_of_global.ofut_enable) {
        CC_LOG_DEBUG(FD_LIST_COUNT_LOG "SETUP PRI PIPE",
                     __FUNCTION__, __LINE__, pollthr_name,
                     g_list_length(thr_pvt_p->fd_list));
    }
    g_private_set(&tname_key,
                  (gpointer)thr_pvt_p);

    free(pollthr_data_p);

    CC_LOG_DEBUG("%s(%d)[%s]: reading on fd %d",
                 __FUNCTION__, __LINE__, pollthr_name,
                 thr_pvt_p->pollfd_arr[0].fd);
    
    /* synchronize completion of thread initialization */
    g_mutex_lock(pollthr_data_p->mgr->adp_thr_init_cv_mutex);
    g_cond_signal(pollthr_data_p->mgr->adp_thr_init_cv_cond);
    g_mutex_unlock(pollthr_data_p->mgr->adp_thr_init_cv_mutex);

    for( ; ; ) {
        CC_LOG_DEBUG("%s(%d)[%s] before poll",
                     __FUNCTION__, __LINE__,
                     pollthr_name);
        
        
        thr_pvt_p = g_private_get(&tname_key);

        if (thr_pvt_p->num_pollfds == 0) {
            /* self destruct */
            CC_LOG_DEBUG("%s(%d)[%s] num_pollfds ZERO. Self Destruct",
                         __FUNCTION__, __LINE__,
                         pollthr_name);
            
            break;
        }
        
        rv = poll(thr_pvt_p->pollfd_arr,
                  thr_pvt_p->num_pollfds,
                  10000);
        
        if (rv == -1) {
            CC_LOG_ERROR("%s(%d)[%s]: poll error %d",
                         __FUNCTION__, __LINE__,pollthr_name,
                         errno);
            break;
        } else if (rv == 0) {
            CC_LOG_DEBUG("%s(%d)[%s]:Timeout occurred! "
                         "No data after 10 seconds",
                         __FUNCTION__, __LINE__, pollthr_name);
            CC_LOG_DEBUG("%s(%d)[%s]: thread was polling on %d number of fds",
                         __FUNCTION__, __LINE__, pollthr_name,
                         thr_pvt_p->num_pollfds);
            for (i = 0; i < thr_pvt_p->num_pollfds; i++) {
                CC_LOG_DEBUG("%s(%d)[%s]thr_pvt_p->pollfd_arr[%d].fd: %d, "
                             "thr_pvt_p->pollfd_arr[%d].events: %d",
                             __FUNCTION__, __LINE__, pollthr_name,
                             i, thr_pvt_p->pollfd_arr[i].fd,
                             i, thr_pvt_p->pollfd_arr[i].events);
            }
        } else {
            thr_pvt_fd_list = thr_pvt_p->fd_list;
            
            g_private_replace(&tname_key,
                              (gpointer)thr_pvt_p);
            
            g_list_foreach(thr_pvt_fd_list, (GFunc)poll_fd_process,
                           (gpointer)pollthr_name);

            g_assert(pollthr_name != NULL);
            CC_LOG_DEBUG("%s(%d)[%s]: listing of updated GList",
                         __FUNCTION__, __LINE__, pollthr_name);

            g_list_foreach(thr_pvt_fd_list, (GFunc)print_fd_list, NULL);
            CC_LOG_DEBUG("%s(%d)[%s]: listing %d items of updated pollfd_arr",
                         __FUNCTION__, __LINE__, pollthr_name,
                         thr_pvt_p->num_pollfds);
            
            thr_pvt_p = g_private_get(&tname_key);
            thr_pvt_p->fd_list = thr_pvt_fd_list;
            for (i = 0; i < thr_pvt_p->num_pollfds; i++) {
                CC_LOG_DEBUG("%s(%d)[%s]: thr_pvt_p->pollfd_arr[%d].fd: %d, "
                             "thr_pvt_p->pollfd_arr[%d].events: %d",
                             __FUNCTION__, __LINE__, pollthr_name,
                             i, thr_pvt_p->pollfd_arr[i].fd,
                             i, thr_pvt_p->pollfd_arr[i].events);
            }
        }
    }
    free(thr_pvt_p->pollfd_arr);
    g_list_free_full(thr_pvt_p->fd_list, (GDestroyNotify)fd_entry_free);
    g_mutex_clear(&thr_pvt_p->send_msg_htbl_lock);
    g_hash_table_destroy(thr_pvt_p->send_msg_htbl);

    
    g_mutex_lock((thr_pvt_p->add_del_pipe_cv_mutex));
    g_cond_signal((thr_pvt_p->add_del_pipe_cv_cond));
    g_mutex_unlock((thr_pvt_p->add_del_pipe_cv_mutex));
    
    free(thr_pvt_p);    
}

uint32_t
adp_thr_mgr_get_num_avail_sockfd(adpoll_thread_mgr_t *this)
{
    CC_LOG_DEBUG("max sockets is %d", this->max_sockets);
    CC_LOG_DEBUG("num sockets is %d", this->num_sockets);
    return (this->max_sockets - this->num_sockets);
}

/* return value: write pipe fd */
int
adp_thr_mgr_get_pri_pipe_wr(adpoll_thread_mgr_t *this)
{
    return(this->pipes_arr[PRI_PIPE_WR_FD]);
}

/* return value: read pipe fd */
int
adp_thr_mgr_get_pri_pipe_rd(adpoll_thread_mgr_t *this)
{
    return(this->pipes_arr[PRI_PIPE_RD_FD]);
}

/* return value: write pipe fd */
int
adp_thr_mgr_get_data_pipe_wr(adpoll_thread_mgr_t *this)
{
    return(this->pipes_arr[DATA_PIPE_WR_FD]);
}


/* return value: read pipe fd */
int
adp_thr_mgr_get_data_pipe_rd(adpoll_thread_mgr_t *this)
{
    return(this->pipes_arr[DATA_PIPE_RD_FD]);
}

