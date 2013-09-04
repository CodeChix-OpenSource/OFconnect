#include <glib.h>
#include <stdio.h>
#include "cc_pollthr_mgr.h"
#include <string.h>
#include "cc_of_global.h"
#include "cc_log.h"

#ifndef UNUSED
#define UNUSED __attribute__ ((__unused__))
#endif

#define LIBLOG_SIZE 4096

extern cc_of_global_t cc_of_global;

/* Fixture data */
typedef struct test_data_ {
    adpoll_thread_mgr_t tp_data;    
    char                liblog[LIBLOG_SIZE];
} test_data_t;
    
/*  Function: pollthread_start
 *  This is a fixture funxtion.
 *  Initialize a poll thread with 10 max sockets
 *  and 1 max pipes
 */
static void
pollthread_start(test_data_t *tdata,
                 gconstpointer tudata)
{
    adpoll_thread_mgr_t *temp_mgr_p = NULL;
    char *temp_liblog = NULL;

    /* initialize and setup debug and logfile */
    cc_of_global.oflog_fd = NULL;
    cc_of_global.oflog_file = malloc(sizeof(char) *
                                     LOG_FILE_NAME_SIZE);
    g_mutex_init(&cc_of_global.oflog_lock);
//    cc_of_debug_toggle(TRUE);    //enable if debugging test code
    cc_of_log_toggle(TRUE);
    cc_of_global.ofut_enable = TRUE;
    
    /* create new thread manager */
    temp_mgr_p = adp_thr_mgr_new((char *)tudata, 10, 1);
    
    /* need to copy the contents to tp because tp is
       allocated by test framework - the location needs
       to be preserved and not over-written.
    */
    // bad idea - save the pointer to mgr instead
    g_memmove(&tdata->tp_data, temp_mgr_p, sizeof(adpoll_thread_mgr_t));

    g_test_message("add del mutex %p cv %p", tdata->tp_data.add_del_pipe_cv_mutex,
                   tdata->tp_data.add_del_pipe_cv_cond);                   
    
    //use cc_of_log_read() to read the contents of log file
    temp_liblog = cc_of_log_read();

    if (temp_liblog) {
        g_test_message("file content size: %u",
                       (uint)strlen(temp_liblog));
        memcpy(tdata->liblog, temp_liblog, LIBLOG_SIZE);
    
        g_free(temp_liblog);
    }
    
    g_free(temp_mgr_p);

}

/* Function that tears down the test and cleans up */
static void
pollthread_end(test_data_t *tdata,
               gconstpointer tudata UNUSED)
{
    cc_of_log_toggle(FALSE);
    cc_of_global.ofut_enable = FALSE;

    g_test_message("In POLLTHREAD_END");
    
    g_test_message("add del mutex %p cv %p", tdata->tp_data.add_del_pipe_cv_mutex,
                   tdata->tp_data.add_del_pipe_cv_cond);                   
    
    adp_thr_mgr_free(&(tdata->tp_data));
    g_mutex_clear(&cc_of_global.oflog_lock);
    g_free(cc_of_global.oflog_file);
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

/* check the basic health upon bringing up poll thread */
static void
pollthread_tc_1(test_data_t *tdata,
                gconstpointer tudata)
{
    if (tdata != NULL) {
        g_test_message("test - name of thread is %s", (char *)tudata);
        g_assert_cmpstr(tdata->tp_data.tname, ==, (char *)tudata);

        g_test_message("add del mutex %p cv %p", tdata->tp_data.add_del_pipe_cv_mutex,
                       tdata->tp_data.add_del_pipe_cv_cond);                   
        
        g_test_message("test - 2 pipes created; 4 pipe fds");
        g_test_message("test - num_pipes %d", tdata->tp_data.num_pipes);
        g_assert_cmpuint(tdata->tp_data.num_pipes, ==, 4);
        
        g_test_message("test - num_avail_sockfd is 10");
        g_assert_cmpint(adp_thr_mgr_get_num_avail_sockfd(&tdata->tp_data),
                        ==, 10);

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
            1, adp_thr_mgr_get_pri_pipe_rd(&tdata->tp_data));
        
        g_test_message("test - value of primary pipe write fd");
        regex_one_compint(
            tdata->liblog,
            "pipe fds created.*([0-9])..([0-9])..PRIMARY",
            2, adp_thr_mgr_get_pri_pipe_wr(&tdata->tp_data));
        

        g_test_message("test - num fd_entry_p in fd_list is 2");        
        regex_one_compint(
            tdata->liblog,
            "fd_list has ([0-9]) entries ADD_FD",
            1, 2);

        g_test_message("test - value of data pipe read fd");
        regex_one_compint(
            tdata->liblog,
            "pipe fds created.*([0-9])..([0-9])..ADD-ON",
            1, adp_thr_mgr_get_data_pipe_rd(&tdata->tp_data));

        g_test_message("test - value of data pipe write fd");
        regex_one_compint(
            tdata->liblog,
            "pipe fds created.*([0-9])..([0-9])..ADD-ON",
            2, adp_thr_mgr_get_data_pipe_wr(&tdata->tp_data));


        g_test_message("test - num_pollfds");
        regex_one_compint(
            tdata->liblog,
            "num pollfds is ([0-9]+) after ADD_FD",
            1, 2);

    } else {
        g_test_message("fixture data invalid");
        g_test_fail();
    }
}


/* pollin function for new pipe with newly defined message
 *  definition
 */
typedef struct test_pipe_in_data_ {
    char msg[32];
} test_pipe_in_data_t;

void
test_pipe_in_process_func(char *tname,
                          adpoll_fd_info_t *data_p,
                          adpoll_send_msg_htbl_info_t *unused_data UNUSED)
{
    test_pipe_in_data_t in_data;
    g_test_message("In TEST_PIPE_IN_PROCESS_FUNC");
    g_assert_cmpstr(tname, ==, "thread_tc_2");

    read(data_p->fd, &in_data, sizeof(in_data));
    g_assert_cmpstr(in_data.msg, ==, "hello 1..2..3");
}



//tc_2 - exercise the primary pipe - add/del pipe
//     - create a new pipe, pass a pollin func
//     - delete the pipe
static void
pollthread_tc_2(test_data_t *tdata,
                gconstpointer tudata UNUSED)
{
    adpoll_thr_msg_t add_pipe_msg;
    adpoll_thr_msg_t del_pipe_msg;
    test_pipe_in_data_t test_msg;
    char *temp_liblog = NULL;
    int wr_fd;
    
    /* add_del_fd - add pipe with pollin func */
    /* test pipe */
    /* add_del_fd - delete pipe */
    add_pipe_msg.fd_type = PIPE;
    add_pipe_msg.fd_action = ADD_FD;
    add_pipe_msg.poll_events = POLLIN;
    add_pipe_msg.pollin_func = &test_pipe_in_process_func;
    add_pipe_msg.pollout_func = NULL;

    /* clear the log */
    cc_of_log_clear();
    
    wr_fd = adp_thr_mgr_add_del_fd(&tdata->tp_data, &add_pipe_msg);

    g_assert (wr_fd != -1);
    /* send test message */
    sprintf(test_msg.msg, "hello 1..2..3");

    write(wr_fd, &test_msg, sizeof(test_msg));
    
//
//    g_test_message("test - output of log follows");
//    g_test_message("%s",tdata->liblog);
//    g_test_message("test - output of log ends");

    temp_liblog = cc_of_log_read();

    g_test_message("Test health after add test pipe");

    /* test the health */
    g_test_message("test - 3 pipes created; 6 pipe fds");
    g_test_message("test - num_pipes %d", tdata->tp_data.num_pipes);
    g_assert_cmpuint(tdata->tp_data.num_pipes, ==, 6);
    
    g_test_message("test - num_avail_sockfd is 10");
    g_assert_cmpint(adp_thr_mgr_get_num_avail_sockfd(&tdata->tp_data),
                    ==, 10);
    
    g_test_message("test - num fd_entry_p in fd_list is 3");
    regex_one_compint(
        temp_liblog,
        "fd_list has ([0-9]+) entries ADD_FD",
        1, 3);

    g_test_message("test - num_pollfds");
    regex_one_compint(
        temp_liblog,
        "num pollfds is ([0-9]+) after ADD_FD",
        1, 3);
    
    /* clear the log */
    cc_of_log_clear();

    del_pipe_msg.fd = wr_fd;
    del_pipe_msg.fd_type = PIPE;
    del_pipe_msg.fd_action = DELETE_FD;
    del_pipe_msg.poll_events = 0;
    del_pipe_msg.pollin_func = NULL;
    del_pipe_msg.pollout_func = NULL;

    adp_thr_mgr_add_del_fd(&tdata->tp_data, &del_pipe_msg);

    temp_liblog = cc_of_log_read();    

    g_test_message("Test health after del test pipe");
    
    g_test_message("test - 2 pipes created; 4 pipe fds");
    g_test_message("test - num_pipes %d", tdata->tp_data.num_pipes);
    g_assert_cmpuint(tdata->tp_data.num_pipes, ==, 4);
    
    g_test_message("test - num_avail_sockfd is 10");
    g_assert_cmpint(adp_thr_mgr_get_num_avail_sockfd(&tdata->tp_data),
                    ==, 10);
    
    g_test_message("test - num fd_entry_p in fd_list is 2");
    regex_one_compint(
        temp_liblog,
        "fd_list has ([0-9]+) entries DELETE_FD",
        1, 2);

    g_test_message("test - num_pollfds");
    regex_one_compint(
        temp_liblog,
        "num pollfds is ([0-9]+) after DELETE_FD",
        1, 2);
}


//tc_3 - exercise the primary pipe - add/del socket
//     - create a file, pass a socket
//     - with pollin and pollout func
//     - delete the socket


//tc_4 - exercise the data pipe
//     - first add file socket using pri pipe
//     - then send message on data pipe and test
//     - delete the file and socket

int main(int argc, char **argv)
{
    g_test_init(&argc, &argv, NULL);

    g_test_add("/pollthread/tc_1",
               test_data_t, /* fixture data - no need to allocate, just give the type */
               "thread_tc_1",                /* user data - second argument to the functions */
               pollthread_start, pollthread_tc_1, pollthread_end);


    g_test_add("/pollthread/tc_2",
               test_data_t, /* fixture data - no need to allocate, just give the type */
               "thread_tc_2",                /* user data - second argument to the functions */
               pollthread_start, pollthread_tc_2, pollthread_end);

    return g_test_run();
}
