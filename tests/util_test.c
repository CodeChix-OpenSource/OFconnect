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
** Description:    	Test utilities
** Assumptions:         N/A
** Testing:             N/A
**
** Main Contact:        deepa.dhurka@gmail.com
** Alt. Contact:        organizers@codechix.org
****************************************************************
*/

#include <glib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/un.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

#include "cc_of_util.h"
#include "cc_of_global.h"
#include "cc_log.h"
#include "cc_of_lib.h"
#include "cc_tcp_conn.h"
#include "cc_udp_conn.h"


#ifndef UNUSED
#define UNUSED __attribute__ ((__unused__))
#endif

#define LIBLOG_SIZE 4096

#define MAX_TEST_POLLTHR_LIST_SIZE 10

//extern cc_of_global_t cc_of_global;
extern
gboolean cc_ofrw_htbl_equal_func(gconstpointer a, gconstpointer b);
extern
gboolean cc_ofchannel_htbl_equal_func(gconstpointer a, gconstpointer b);
extern
gboolean cc_ofdev_htbl_equal_func(gconstpointer a, gconstpointer b);
extern
void cc_of_destroy_generic(gpointer data);
extern
void cc_ofdev_htbl_destroy_val(gpointer data);

extern
void process_tcpfd_pollin_func(char *tname,
                               adpoll_fd_info_t *data_p,
                               adpoll_send_msg_htbl_info_t *unused_data);
extern
void process_tcpfd_pollout_func(char *tname,
                                adpoll_fd_info_t *data_p,
                                adpoll_send_msg_htbl_info_t *send_msg_p);


/* Fixture data */
/* tp_data has one rwpoll thread created at init 
 * this should also be available in the cc_of_global list
 * cc_of_global.ofrw_pollthr_list
 */
typedef struct test_data_ {
    adpoll_thread_mgr_t tp_data[MAX_TEST_POLLTHR_LIST_SIZE];
    char                liblog[LIBLOG_SIZE];
} test_data_t;
    
/*  Function: util_start
 *  This is a fixture function.
 *  Initialize one rw pollthread
 * create one rw pollthread
 * create one listenfd pollthread
 */
static void
util_start(test_data_t *tdata,
           gconstpointer tudata UNUSED)
{
    adpoll_thread_mgr_t *temp_mgr_p = NULL;
    char *temp_liblog = NULL;
    cc_of_ret status;

    // Initialize cc_of_global
    cc_of_global.ofut_enable = TRUE;
    cc_of_global.oflog_fd = NULL;
    cc_of_global.oflog_file = malloc(sizeof(char) *
                                     LOG_FILE_NAME_SIZE);
    g_mutex_init(&cc_of_global.oflog_lock);
    g_mutex_init(&cc_of_global.ofdev_htbl_lock);
    g_mutex_init(&cc_of_global.ofchannel_htbl_lock);
    g_mutex_init(&cc_of_global.ofrw_htbl_lock);
    
//    cc_of_debug_toggle(TRUE);    //enable if debugging test code
    cc_of_log_toggle(TRUE);
    
    cc_of_global.oflisten_pollthr_p = NULL;
    cc_of_global.ofrw_pollthr_list = NULL;
    
    cc_of_global.ofdev_type = CONTROLLER;
    cc_of_global.ofdev_htbl = g_hash_table_new_full(cc_ofdev_hash_func,
                                                    cc_ofdev_htbl_equal_func,
                                                    cc_of_destroy_generic,
                                                    cc_ofdev_htbl_destroy_val);
    g_assert(cc_of_global.ofdev_htbl != NULL);


    cc_of_global.ofchannel_htbl = g_hash_table_new_full(cc_ofchann_hash_func,
                                                        cc_ofchannel_htbl_equal_func,
                                                        cc_of_destroy_generic,
                                                        cc_of_destroy_generic);
    g_assert(cc_of_global.ofchannel_htbl != NULL);


    cc_of_global.ofrw_htbl = g_hash_table_new_full(cc_ofrw_hash_func,
                                                   cc_ofrw_htbl_equal_func,
                                                   cc_of_destroy_generic,
                                                   cc_of_destroy_generic);

    g_assert (cc_of_global.ofrw_htbl != NULL);

   
    cc_of_log_clear();
    cc_create_rw_pollthr(&temp_mgr_p);
    g_assert(temp_mgr_p != NULL);
    
    g_memmove(&tdata->tp_data[0], temp_mgr_p, sizeof(adpoll_thread_mgr_t));

    cc_of_global.NET_SVCS[TCP] = tcp_sockfns;
    cc_of_global.NET_SVCS[UDP] = udp_sockfns;

    cc_of_global.oflisten_pollthr_p = adp_thr_mgr_new("oflisten_thr",
                                                      MAX_PER_THREAD_RWSOCKETS,
                                                      MAX_PER_THREAD_PIPES);
    if (cc_of_global.oflisten_pollthr_p == NULL) {
	    status = CC_OF_EMISC;
	    cc_of_lib_free();
        g_assert(1);
    }
   
    temp_liblog = cc_of_log_read();
    
    if (temp_liblog) {
        g_test_message("file content size: %u",
                       (uint)strlen(temp_liblog));
        memcpy(tdata->liblog, temp_liblog, LIBLOG_SIZE);
        
        g_free(temp_liblog);
    }
}

void
free_pollthr_list_elem(adpoll_thread_mgr_t *elem)
{
    free(elem);
}

static void
util_end(test_data_t *tdata UNUSED,
         gconstpointer tudata UNUSED)
{
    cc_of_log_toggle(FALSE);
    cc_of_global.ofut_enable = FALSE;

    g_test_message("In UTIL_END");

    cc_of_lib_free();
}

void
regex_one_compint(char *liblog, //size LIBLOG_SIZE
                  char *pattern,
                  int match_iter,
                  int compareval)
{
    GRegex *regex = NULL;
    GMatchInfo *match_info;

    regex = g_regex_new (pattern, G_REGEX_MULTILINE,
                         0, NULL);
    if (!(g_regex_match(regex, liblog, 0, &match_info))) {
        g_test_message("no successful match");
        g_test_fail();
    }
        
    if (g_match_info_matches (match_info))
    {
        gchar *word = g_match_info_fetch (match_info, match_iter);
        g_assert_cmpint(atoi(word), ==, compareval);
        g_free (word);
    }
    g_match_info_free (match_info);
    g_regex_unref (regex);
}

// check the basic health upon initializing the ofrw_pollthr_list
// test the fixture data is setup correctly
// test the basic health of primary and data pipe
// test the tear down of poll thread (self destruct)
// test the synchronization in adding/deleting fds
static void
util_tc_1(test_data_t *tdata, gconstpointer tudata)
{
    GList *first_elem_list = NULL;
    adpoll_thread_mgr_t *list_elem = NULL;
    
    g_test_message("test - name of thread is %s", (char *)tudata);
    g_assert_cmpstr(tdata->tp_data[0].tname, ==, (char *)tudata);
    
    g_test_message("add del mutex %p cv %p", tdata->tp_data[0].add_del_pipe_cv_mutex,
                   tdata->tp_data[0].add_del_pipe_cv_cond);                   
    
    g_test_message("test - 2 pipes created; 4 pipe fds");
    g_test_message("test - num_pipes %d", tdata->tp_data[0].num_pipes);
    g_assert_cmpuint(tdata->tp_data[0].num_pipes, ==, 4);
    
    g_test_message("test - num_avail_sockfd is MAX_PER_THREAD_RWSOCKETS");
    g_assert_cmpint(adp_thr_mgr_get_num_avail_sockfd(&tdata->tp_data[0]),
                    ==, MAX_PER_THREAD_RWSOCKETS);
    
    g_test_message("test - output of log follows");
    g_test_message("%s",tdata->liblog);
    g_test_message("test - output of log ends");
    
    g_test_message("test - num fd_entry_p in fd_list is 1");
    regex_one_compint(
        tdata->liblog,
        "fd_list has ([0-9]+) entries SETUP PRI PIPE",
        1, 1);
    
    g_test_message("test - value of primary pipe read fd");
    regex_one_compint(
        tdata->liblog,
        "pipe fds created.*([0-9])..([0-9])..PRIMARY",
        1, adp_thr_mgr_get_pri_pipe_rd(&tdata->tp_data[0]));
    
    g_test_message("test - value of primary pipe write fd");
    regex_one_compint(
        tdata->liblog,
        "pipe fds created.*([0-9])..([0-9])..PRIMARY",
        2, adp_thr_mgr_get_pri_pipe_wr(&tdata->tp_data[0]));
    
    
    g_test_message("test - num fd_entry_p in fd_list is 2");        
    regex_one_compint(
        tdata->liblog,
        "fd_list has ([0-9]) entries ADD_FD",
        1, 2);
    
    g_test_message("test - value of data pipe read fd");
    regex_one_compint(
        tdata->liblog,
        "pipe fds created.*([0-9])..([0-9])..ADD-ON",
        1, adp_thr_mgr_get_data_pipe_rd(&tdata->tp_data[0]));
    
    g_test_message("test - value of data pipe write fd");
    regex_one_compint(
        tdata->liblog,
        "pipe fds created.*([0-9])..([0-9])..ADD-ON",
        2, adp_thr_mgr_get_data_pipe_wr(&tdata->tp_data[0]));
    
    g_test_message("test - num_pollfds");
    regex_one_compint(
        tdata->liblog,
        "num pollfds is ([0-9]+) after ADD_FD",
        1, 2);
    
    
    // now compare the global list element to the fixture data
    g_test_message("test - sanity of element in ofrw_pollthr_list");

    g_assert(g_list_length(cc_of_global.ofrw_pollthr_list) == 1);
    first_elem_list = g_list_first(cc_of_global.ofrw_pollthr_list);
    list_elem = (adpoll_thread_mgr_t *)first_elem_list->data;
    
    g_assert_cmpstr(list_elem->tname, ==, tdata->tp_data[0].tname);
    g_assert_cmpuint(list_elem->num_sockets, ==, tdata->tp_data[0].num_sockets);
    g_assert_cmpuint(list_elem->num_pipes, ==, tdata->tp_data[0].num_pipes);
    g_assert_cmpuint(list_elem->max_sockets, ==, tdata->tp_data[0].max_sockets);
    g_assert_cmpuint(list_elem->max_pipes, ==, tdata->tp_data[0].max_pipes);
    
}

gboolean fd_is_unique(gint32 fd_arr[], gint32 fd_arr_size, gint32 fd)
{
    int i;
    
    if (fd_arr_size == 0) {
        return TRUE;
    }

    for (i = 0; i < fd_arr_size; i++) {
        if (fd_arr[i] == fd) {
            return TRUE;
        }
    }
    
    return FALSE;
}

/* we are not testing this functionality here */
/* therefore using a noop function for dev register */
int dummy_recv_func(uint64_t dp_id UNUSED,
                    uint8_t aux_id UNUSED,
                    void *of_msg UNUSED,
                    size_t of_msg_len UNUSED)
{
    return 1;
}

int dummy_accept_func(uint64_t dummy_dpid UNUSED,
                      uint8_t dummy_auxid UNUSED)
{
    CC_LOG_DEBUG("%s(%d) SOCKET ACCEPTED CALLBACK", __FUNCTION__,
                 __LINE__);
    return 1;
}

int dummy_del_func(uint64_t dpid UNUSED,
                   uint8_t auxid UNUSED)
{
    return 1;
}



char payload_str[] = "util_tc_2: rwsocket under test message";
//char tc3_test_str_out[] = "util_tc_2: socket out test message";

//util_tc_2
// tc_2 + test tcp net svcs pollin and pollout functions
//
// details:
// create a socket pair
// add 1 rwsocket
// exercise tcp pollin and pollout and listen pollin
// delete the rwsocket
// test connect to listen socket and accept new conn
// test pollout and pollin func on rwsocket establised
// test dev_free
// test TCP net svcs listen accept


//Expected: 3 poll fds on oflisten_thr
//(2 primary + 1 listen fd)
//Expected: 4 poll fds on ofrw_thr
//(2 primary + 1 connect + 1 accept + 1 udp if enabled)

int
receive_packet_process_func(uint64_t dp_id, uint8_t aux_id,
                            void *of_msg, 
                            size_t of_msg_len)
{
    char in_str[100];
    g_test_message("test - %s: message received by receive process CALLBACK API "
                   "is \"%s\"", __FUNCTION__, payload_str);

    if (of_msg == NULL) {
        CC_LOG_ERROR("%s(%d): null message received", __FUNCTION__,
                     __LINE__);
    }
    if (of_msg_len == 0) {
        CC_LOG_DEBUG("%s(%d): zero sized message arrived for dp/aux %lu/%hu",
                    __FUNCTION__, __LINE__, dp_id, aux_id);
        return 0;
    } else {
        g_memmove(in_str,of_msg,of_msg_len);
        CC_LOG_DEBUG("size of message is %d", of_msg_len);
        in_str[of_msg_len] = 0;
        CC_LOG_DEBUG("%s: WOOOHOOOO message received %s", __FUNCTION__,
                     in_str);
    }
    return 0;
}

//register the recv pkt callback
// do not use fixture data
static void
util_tc_2(test_data_t *tdata UNUSED, gconstpointer tudata)
{

    gint32 dummy_fd;
    gint32 num_fds = 0;
    cc_ofdev_key_t devkey;
    cc_of_ret retval;
    char send_buf[1024];
    int clientfd;
    cc_of_ret client_status = CC_OF_OK;
    int optval = 1;
    struct sockaddr_in serveraddr, localaddr;
    cc_ofchannel_key_t ofchann_key;
    adpoll_thread_mgr_t *tmgr = NULL;
    int datapipe_fd;
    int sockflags;
    
//    cc_of_debug_toggle(TRUE);    //enable if debugging test code    
    /* device setup */
    /* ip address 127.0.0.1 */
    devkey.controller_ip_addr = 0x7F000001;
    devkey.switch_ip_addr = 0x7F000001;
    devkey.controller_L4_port = 6633;

    retval = cc_of_dev_register(devkey.controller_ip_addr,
                                devkey.switch_ip_addr,
                                devkey.controller_L4_port,
                                CC_OFVER_1_3_1,
                                receive_packet_process_func,
                                dummy_accept_func,
                                dummy_del_func);
    g_test_message("retval from dev register is CC_OF_OK");
    g_assert(retval == CC_OF_OK);

    // wait till listen is up
    g_usleep(100000); //1 million microseconds is 1 sec
    
    //start a connection and try to connect with listen fd
    //socket(); connect() - use the code in cc_tcp_conn
    //manually establish client fd for writing
    {
    ofchann_key.dp_id = 101001000;
    ofchann_key.aux_id = 0;

    if ((clientfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        CC_LOG_ERROR("%s(%d): %s", __FUNCTION__, __LINE__,
                     strerror(errno));
    }

    CC_LOG_DEBUG("%s(%d): NEW CLIENT SOCKET %d",
                 __FUNCTION__, __LINE__, clientfd);

    // To prevent "Address already in use" error from bind
    if ((client_status = setsockopt(clientfd, SOL_SOCKET,SO_REUSEADDR, 
                             (const void *)&optval, sizeof(int))) < 0) {
        CC_LOG_ERROR("%s(%d): %s", __FUNCTION__, __LINE__, 
                     strerror(errno));
    }

    memset(&localaddr, 0, sizeof(localaddr));
    localaddr.sin_family = AF_INET;
    localaddr.sin_addr.s_addr = htonl(devkey.switch_ip_addr);
    localaddr.sin_port = 0;
    
    // Bind clienfd to a local interface addr
    if ((client_status = bind(clientfd, (struct sockaddr *) &localaddr, 
                       sizeof(localaddr))) < 0) {
	    CC_LOG_ERROR("%s(%d): %s", __FUNCTION__, __LINE__, strerror(errno));
    }
 
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(devkey.controller_ip_addr);
    serveraddr.sin_port = htons(devkey.controller_L4_port);

    // Establish connection with server
    if ((client_status = connect(clientfd, (struct sockaddr *) &serveraddr, 
                                 sizeof(serveraddr))) < 0) {
        CC_LOG_ERROR("%s(%d): %s", __FUNCTION__, __LINE__, strerror(errno));
    }

    sockflags = fcntl(clientfd,F_GETFL,0);
    g_assert(sockflags != -1);
    fcntl(clientfd, F_SETFL, sockflags | O_NONBLOCK);

    // Add clientfd to a thr_mgr and update it in ofrw, ofdev htbls
    // we don't need to do this step if we use create_channel api
    adpoll_thr_msg_t thr_msg;

    thr_msg.fd = clientfd;
    thr_msg.fd_type = SOCKET;
    thr_msg.fd_action = ADD;
    thr_msg.poll_events = POLLIN | POLLOUT;
    thr_msg.pollin_func = &process_tcpfd_pollin_func;
    thr_msg.pollout_func = &process_tcpfd_pollout_func;
    
    client_status = cc_add_sockfd_rw_pollthr(&thr_msg, devkey, TCP, ofchann_key);
    if (client_status < 0) {
        CC_LOG_ERROR("%s(%d):Error updating tcp sockfd in global structures: %s",
                     __FUNCTION__, __LINE__, cc_of_strerror(errno));
        close(clientfd);
    }
    
    }
    
    CC_LOG_DEBUG("zzzzzzzzzzzzzzzzzzzzzz");
    /* sleep for sometime to allow the polling thread
       to process the receive the connection and accept
    */
    g_usleep(2000000); //1 million microseconds is 1 sec
    CC_LOG_DEBUG("zzzzzzzzzzzzzzzzzzzzzz");


    /* now write to the client FD and test on pollin for RW FD */
    ((adpoll_send_msg_t *)send_buf)->hdr.msg_size =
        sizeof(adpoll_send_msg_t) + strlen(payload_str) + 1;
    
    ((adpoll_send_msg_t *)send_buf)->hdr.fd = clientfd;
    
    g_memmove(((adpoll_send_msg_t *)send_buf)->data,
              payload_str, strlen(payload_str) + 1);

    find_thrmgr_rwsocket_lockfree(clientfd, &tmgr);

    g_assert_cmpstr(tmgr->tname, ==, "rwthr_1");

    CC_LOG_DEBUG("%s(%d) found the rw thread for client sockfd %d",
                 __FUNCTION__, __LINE__, clientfd);

    if (tmgr == NULL) {
        CC_LOG_ERROR("%s(%d): socket %d is invalid",
                     __FUNCTION__, __LINE__, clientfd);
    }
    datapipe_fd = adp_thr_mgr_get_data_pipe_wr(tmgr);
    
    write(datapipe_fd, send_buf,
          ((adpoll_send_msg_t *)send_buf)->hdr.msg_size);

    CC_LOG_DEBUG("%s(%d): wrote to data pipe %d", __FUNCTION__, __LINE__, datapipe_fd);

    CC_LOG_DEBUG("%s====SNOOZE FOR MESSAGE TO PLUMB THROUGH====",
                 __FUNCTION__);
    
    g_usleep(2000000);    
    CC_LOG_DEBUG("%s - ALL DONE", __FUNCTION__);
}


int main(int argc, char **argv)
{
    g_test_init(&argc, &argv, NULL);

    g_test_add("/util/tc_1",
               test_data_t,
               "rwthr_1",
               util_start, util_tc_1, util_end);

    g_test_add("/util/tc_2",
               test_data_t,
              "rwthr_1",
               util_start, util_tc_2, util_end);
    
    return g_test_run();
}

