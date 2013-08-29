#include <glib.h>
#include <stdio.h>
#include "cc_pollthr_mgr.h"
#include <string.h>

#ifndef UNUSED
#define UNUSED __attribute__ ((__unused__))
#endif

//static adpoll_thread_mgr_t *tp;
/*  Function: pollthread_init
 *  initialize a poll thread with 10 max sockets
 *  and 0 max pipes
 */
static void
pollthread_start(adpoll_thread_mgr_t *tp,
                 gconstpointer tdata UNUSED)
{
    adpoll_thread_mgr_t *temp_mgr_p;
    temp_mgr_p = adp_thr_mgr_new("test_adp", 10, 0);

    /* need to copy the contents to tp because tp is
       allocated by test framework - the location needs
       to be preserved and not over-written.
    */
    g_memmove(tp, temp_mgr_p, sizeof(adpoll_thread_mgr_t));

    g_free(temp_mgr_p);
    g_assert(tp != NULL);
    g_assert_cmpstr(tp->tname, ==, "test_adp");
}

static void
pollthread_end(adpoll_thread_mgr_t *tp,
               gconstpointer tdata UNUSED)
{
    adp_thr_mgr_free(tp);
}

static void
pollthread_init(adpoll_thread_mgr_t *tp,
                gconstpointer tdata UNUSED)
{
    g_test_message("test - thread manager not NULL");
    g_assert(tp != NULL);

    g_test_message("test - name of thread is test_adp");
    g_assert_cmpstr(tp->tname, ==, "test_adp");

    g_test_message("test - 2 pipes created; 4 pipe fds");
    g_test_message("test - num_pipes %d", tp->num_pipes);
    g_assert(tp->num_pipes == 4);

    g_test_message("test - num_avail_sockfd is 10");
    g_assert(adp_thr_mgr_get_num_avail_sockfd(tp) == 10);

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
               adpoll_thread_mgr_t, /* fixture data - no need to allocate, just give the type */
               NULL,                /* user data - second argument to the functions */
               pollthread_start, pollthread_init, pollthread_end);

    return g_test_run();
}
