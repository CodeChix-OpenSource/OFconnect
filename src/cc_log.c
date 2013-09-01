/*-----------------------------------------------------------------------------*/
/* Copyright: CodeChix Bay Area Chapter 2013                                   */
/*-----------------------------------------------------------------------------*/
#include "cc_log.h"
#include "cc_of_global.h"

extern cc_of_global_t cc_of_global;

static char *
get_time_stamp(){
    
    char *tstamp = malloc(sizeof(char) * 16);
    time_t ltime;
    struct tm *tm;
    
    ltime=time(NULL);
    tm=localtime(&ltime);
    
    sprintf(tstamp,"%04d-%02d-%02d-%02d%02d%02d",
            tm->tm_year+1900, tm->tm_mon, 
            tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
    return tstamp;
}

/* return value: new file fd
   argument returns path and name of log file
   creates a directory .libccof in $HOME
   creates a new file with log-<timestamp> name
*/
FILE *
create_logfile(char *logfile)
{
    char *logdir;
    FILE *logfd = NULL;

    if (logfile == NULL) {
        CC_LOG_ERROR("%s(%d): Memory not allocated for logfile",
                     __FUNCTION__, __LINE__);
        return NULL;
    }
    
    logdir =  malloc(sizeof(char) * 144);
    sprintf(logdir,"%s/.libccof", getenv("HOME"));
    sprintf(logfile,"%s/log-%s", logdir, get_time_stamp());

    if (g_mkdir_with_parents(logdir, S_IFDIR | S_IRWXU | S_IRWXG | S_IROTH)) {
        printf("error - %s\n", g_strerror(errno));
    }

    logfd = g_fopen(logfile,"a+");
    return logfd;
}

/* MUST Remember to call g_free for returned pointer */
char *
read_logfile(char *logfile)
{
    char *logdata = NULL;
    GError *logerr = NULL;
    g_file_get_contents (logfile, &logdata, NULL, &logerr);
    
    g_assert ((logdata == NULL && logerr != NULL) ||
              (logdata != NULL && logerr == NULL));
    
    if (logerr != NULL) {
        /* Report error to user, and free error */
        CC_LOG_ERROR("%s(%d): Unable to read file: %s\n",
                     __FUNCTION__, __LINE__, logerr->message);
        g_error_free(logerr);
    }
    return logdata;
}

void
write_logfile_lock(char *msg)
{
    g_mutex_lock(&cc_of_global.oflog_lock);
    if (cc_of_global.oflog_enable) {
        fprintf(cc_of_global.oflog_fd, "%s", msg);
    }
    g_mutex_unlock(&cc_of_global.oflog_lock);
}
