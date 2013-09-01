#include <glib.h>
#include <stdio.h>
#include "cc_pollthr_mgr.h"
#include <string.h>
#include "cc_of_global.h"
#include "cc_log.h"

#ifndef UNUSED
#define UNUSED __attribute__ ((__unused__))
#endif

#define LIBLOG_SIZE 1024

extern cc_of_global_t cc_of_global;

typedef struct test_data_ {
    adpoll_thread_mgr_t tp;
    char                liblog[LIBLOG_SIZE];
} test_data_t;
    
/*  Function: pollthread_init
 *  initialize a poll thread with 10 max sockets
 *  and 0 max pipes
 */

static void
pollthread_start(test_data_t *tdata,
                 gconstpointer tudata UNUSED)
{
    adpoll_thread_mgr_t *temp_mgr_p = NULL;
    char *temp_liblog = NULL;

    /* initialize and setup debug and logfile */
    cc_of_global.oflog_fd = NULL;
    cc_of_global.oflog_file = malloc(sizeof(char) *
                                     LOG_FILE_NAME_SIZE);
    g_mutex_init(&cc_of_global.oflog_lock);
    cc_of_debug_toggle(TRUE);
    cc_of_log_toggle(TRUE);

    
    /* create new thread manager */
    temp_mgr_p = adp_thr_mgr_new("test_adp", 10, 0);

     /* need to copy the contents to tp because tp is
        allocated by test framework - the location needs
        to be preserved and not over-written.
     */
     g_memmove(&tdata->tp, temp_mgr_p, sizeof(adpoll_thread_mgr_t));


     //use cc_of_log_read() to read the contents of log file
     temp_liblog = cc_of_log_read();
     memcpy(tdata->liblog, temp_liblog, sizeof(LIBLOG_SIZE));

     g_free(temp_liblog);
    g_assert(tdata->liblog != NULL);

    g_free(temp_mgr_p);
}

static void
pollthread_end(test_data_t *tdata,
               gconstpointer tudata UNUSED)
{
    adp_thr_mgr_free(&tdata->tp);
}

static void
pollthread_init(test_data_t *tdata,
                gconstpointer tudata UNUSED)
{
    g_test_message("test - thread manager not NULL");
    g_assert(&tdata->tp != NULL);

    g_test_message("test - name of thread is test_adp");
    g_assert_cmpstr(tdata->tp.tname, ==, "test_adp");

    g_test_message("test - 2 pipes created; 4 pipe fds");
    g_test_message("test - num_pipes %d", tdata->tp.num_pipes);
    g_assert(tdata->tp.num_pipes == 4);

    g_test_message("test - num_avail_sockfd is 10");
    g_assert(adp_thr_mgr_get_num_avail_sockfd(&tdata->tp) == 10);

    g_test_message("test - output of log follows");
    g_test_message("%s",tdata->liblog);
    /* use regex features of glib */
    //get pri pipe - compare with log
    //get data pipe - compare with log

    // hack to get pollthr_private info
    // hack to get adpoll_fd_info for each fd
    // hack adpoll_thread_mgr to get data

}

int main(int argc, char **argv)
{
    g_test_init(&argc, &argv, NULL);

    g_test_add("/pollthread/init",
               test_data_t, /* fixture data - no need to allocate, just give the type */
               NULL,                /* user data - second argument to the functions */
               pollthread_start, pollthread_init, pollthread_end);

    return g_test_run();
}
