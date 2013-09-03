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
    adpoll_thread_mgr_t tp;
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
    g_memmove(&tdata->tp, temp_mgr_p, sizeof(adpoll_thread_mgr_t));
    
    
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
    adp_thr_mgr_free(&tdata->tp);
    g_mutex_clear(&cc_of_global.oflog_lock);
    g_free(cc_of_global.oflog_file);
}

void
regex_one_compint(test_data_t *tdata,
              char *pattern,
              int match_iter,
              int compareval)
{
    GRegex *regex = NULL;
    GMatchInfo *match_info;
    
    regex = g_regex_new (pattern, G_REGEX_MULTILINE,
                         0, NULL);
    if (!(g_regex_match(regex, tdata->liblog, 0, &match_info))) {
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
        g_test_message("test - thread manager not NULL");
        g_assert(&tdata->tp != NULL);
        
        g_test_message("test - name of thread is %s", (char *)tudata);
        g_assert_cmpstr(tdata->tp.tname, ==, (char *)tudata);
        
        g_test_message("test - 2 pipes created; 4 pipe fds");
        g_test_message("test - num_pipes %d", tdata->tp.num_pipes);
        g_assert_cmpuint(tdata->tp.num_pipes, ==, 4);
        
        g_test_message("test - num_avail_sockfd is 10");
        g_assert_cmpint(adp_thr_mgr_get_num_avail_sockfd(&tdata->tp),
                        ==, 10);
#if 0
        g_test_message("test - output of log follows");
        g_test_message("%s",tdata->liblog);
        g_test_message("test - output of log ends");
#endif

        g_test_message("test - num fd_entry_p in fd_list is 1");
        regex_one_compint(
            tdata,
            "fd_list has ([0-9]+) entries SETUP PRI PIPE",
            1, 1);


        g_test_message("test - value of primary pipe read fd");
        regex_one_compint(
            tdata,
            "pipe fds created.*([0-9]).*([0-9]).*PRIMARY",
            1, adp_thr_mgr_get_pri_pipe_rd(&tdata->tp));
        
        g_test_message("test - value of primary pipe write fd");
        regex_one_compint(
            tdata,
            "pipe fds created.*([0-9]).*([0-9]).*PRIMARY",
            2, adp_thr_mgr_get_pri_pipe_wr(&tdata->tp));
        

        g_test_message("test - value of data pipe read fd");
        regex_one_compint(
            tdata,
            "pipe fds created.*([0-9]).*([0-9]).*ADD-ON",
            1, adp_thr_mgr_get_data_pipe_rd(&tdata->tp));


        g_test_message("test - value of data pipe write fd");
        regex_one_compint(
            tdata,
            "pipe fds created.*([0-9]).*([0-9]).*ADD-ON",
            2, adp_thr_mgr_get_data_pipe_wr(&tdata->tp));

#if 0
        //notes
        adpoll_fd_info_t *fd_entry_p; << thr_pvt->fd_list refcount
                                             pollthr_private_t *thr_pvt_p; << num_pollfds
            typedef struct pollthr_private_ {
                                                 int           num_pollfds;
                                                 struct pollfd *pollfd_arr;
                                                 GList         *fd_list;
                                                 GMutex        *del_pipe_cv_mutex_p;
                                                 GCond         *del_pipe_cv_cond_p;
                                             } pollthr_private_t;
#endif                                    
        
    }

}

//tc_2 - create a new pipe, pass a pollin func
//     - delete the pipe
static void
pollthread_tc_2(test_data_t *tdata UNUSED,
                gconstpointer tudata UNUSED)
{

}
#if 0
    /* create a pipe */
    if (tdata != NULL) {
        g_test_message("test - thread manager not NULL");
        g_assert(&tdata->tp != NULL);
        
        g_test_message("test - name of thread is test_adp");
        g_assert_cmpstr(tdata->tp.tname, ==, "test_adp");
        
        g_test_message("test - 2 pipes created; 4 pipe fds");
        g_test_message("test - num_pipes %d", tdata->tp.num_pipes);
        g_assert_cmpuint(tdata->tp.num_pipes, ==, 4);
        
        g_test_message("test - num_avail_sockfd is 10");
        g_assert_cmpint(adp_thr_mgr_get_num_avail_sockfd(&tdata->tp),
                        ==, 10);

        g_test_message("test - output of log follows");
        g_test_message("%s",tdata->liblog);
        g_test_message("test - output of log ends");


        g_test_message("test - num fd_entry_p in fd_list is 1");
        regex_one_compint(
            tdata,
            "fd_list has ([0-9]+) entries SETUP PRI PIPE",
            1, 1);


        g_test_message("test - value of primary pipe read fd");
        regex_one_compint(
            tdata,
            "pipe fds created.*([0-9]).*([0-9]).*PRIMARY",
            1, adp_thr_mgr_get_pri_pipe_rd(&tdata->tp));
        
        g_test_message("test - value of primary pipe write fd");
        regex_one_compint(
            tdata,
            "pipe fds created.*([0-9]).*([0-9]).*PRIMARY",
            2, adp_thr_mgr_get_pri_pipe_wr(&tdata->tp));
        

        g_test_message("test - value of data pipe read fd");
        regex_one_compint(
            tdata,
            "pipe fds created.*([0-9]).*([0-9]).*ADD-ON",
            1, adp_thr_mgr_get_data_pipe_rd(&tdata->tp));


        g_test_message("test - value of data pipe write fd");
        regex_one_compint(
            tdata,
            "pipe fds created.*([0-9]).*([0-9]).*ADD-ON",
            2, adp_thr_mgr_get_data_pipe_wr(&tdata->tp));


        adpoll_fd_info_t *fd_entry_p; << thr_pvt->fd_list refcount
                                             pollthr_private_t *thr_pvt_p; << num_pollfds
                                                                                  typedef struct pollthr_private_ {
                                                 int           num_pollfds;
                                                 struct pollfd *pollfd_arr;
                                                 GList         *fd_list;
                                                 GMutex        *del_pipe_cv_mutex_p;
                                                 GCond         *del_pipe_cv_cond_p;
                                             } pollthr_private_t;

        
        // hack to get pollthr_private info
        // hack to get adpoll_fd_info for each fd
        // hack adpoll_thread_mgr to get data
    }

}
#endif

//tc_3 - create a file, pass a socket and function
//     -  delete one pipe and one socket and check ref count


//tc_4 - create a file, pass a socket and a pollout func
//     - delete the file and socket and check ref count

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
               pollthread_start, pollthread_tc_1, pollthread_end);

    return g_test_run();
}
