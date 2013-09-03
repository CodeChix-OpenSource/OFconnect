/*-----------------------------------------------------------------------------*/
/* Copyright: CodeChix Bay Area Chapter 2013                                   */
/*-----------------------------------------------------------------------------*/
#ifndef CC_LOG_H
#define CC_LOG_H

#include <glib.h>
#include <glib/gprintf.h>
#include <glib/gstdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#define CC_LOG_DOMAIN    ("cc_onf_driver_lib")

#define LOG_FILE_NAME_SIZE         160

#define LOG_MSG_SIZE               400 /* 5 lines' worth */

FILE *
create_logfile(char *logfile);

/* reads without locking
 * wrapper with locking in cc_of_lib.h
 */
char *
read_logfile(void);

/* write inside locks */
void
write_logfile_lock(char *msg);

#define CC_LOG_ENABLE_DEBUGS()                     \
    {                                              \
        g_setenv("G_MESSAGES_DEBUG", "all", TRUE); \
    }

#define CC_LOG_DISABLE_DEBUGS()                    \
    {                                              \
        g_unsetenv("G_MESSAGES_DEBUG");            \
    }

#define CC_LOG_WARNING(...)                                             \
    {                                                                   \
        char lmsg[LOG_MSG_SIZE];                                        \
        char lmsg_temp[LOG_MSG_SIZE];                                   \
                                                                        \
        g_log(CC_LOG_DOMAIN, G_LOG_LEVEL_WARNING, __VA_ARGS__);         \
                                                                        \
        sprintf(lmsg_temp, __VA_ARGS__);                                \
        sprintf(lmsg,"%s %d:", CC_LOG_DOMAIN, G_LOG_LEVEL_WARNING);     \
        strcat(lmsg, lmsg_temp);                                        \
                                                                        \
        write_logfile_lock(lmsg);                                       \
    }

#define CC_LOG_DEBUG_NOLOG(...)                                         \
    {                                                                   \
        g_log(CC_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, __VA_ARGS__);           \
    }

#define CC_LOG_DEBUG(...)                                               \
    {                                                                   \
        char lmsg[LOG_MSG_SIZE];                                        \
        char lmsg_temp[LOG_MSG_SIZE];                                   \
                                                                        \
        g_log(CC_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, __VA_ARGS__);           \
                                                                        \
        sprintf(lmsg_temp, __VA_ARGS__);                                \
        sprintf(lmsg,"%s %d:", CC_LOG_DOMAIN, G_LOG_LEVEL_DEBUG);       \
        strcat(lmsg, lmsg_temp);                                        \
                                                                        \
        write_logfile_lock(lmsg);                                       \
    }


#define CC_LOG_ERROR(...)                                               \
    {                                                                   \
        char lmsg[LOG_MSG_SIZE];                                        \
        char lmsg_temp[LOG_MSG_SIZE];                                   \
                                                                        \
        g_log(CC_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, __VA_ARGS__);        \
                                                                        \
        sprintf(lmsg_temp, __VA_ARGS__);                                \
        sprintf(lmsg,"%s %d:", CC_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL);    \
        strcat(lmsg, lmsg_temp);                                        \
                                                                        \
        write_logfile_lock(lmsg);                                       \
    }


#define CC_LOG_INFO(...)                                                \
    {                                                                   \
        char lmsg[LOG_MSG_SIZE];                                        \
        char lmsg_temp[LOG_MSG_SIZE];                                   \
                                                                        \
        g_log(CC_LOG_DOMAIN, G_LOG_LEVEL_INFO, __VA_ARGS__);            \
                                                                        \
        sprintf(lmsg_temp, __VA_ARGS__);                                \
        sprintf(lmsg,"%s %d:", CC_LOG_DOMAIN, G_LOG_LEVEL_INFO);        \
        strcat(lmsg, lmsg_temp);                                        \
                                                                        \
        write_logfile_lock(lmsg);                                       \
    }


#define CC_LOG_FATAL(...)                                               \
    {                                                                   \
        char lmsg[LOG_MSG_SIZE];                                        \
        char lmsg_temp[LOG_MSG_SIZE];                                   \
                                                                        \
        g_log(CC_LOG_DOMAIN, G_LOG_LEVEL_ERROR, __VA_ARGS__);           \
                                                                        \
        sprintf(lmsg_temp, __VA_ARGS__);                                \
        sprintf(lmsg,"%s %d:", CC_LOG_DOMAIN, G_LOG_LEVEL_ERROR);       \
        strcat(lmsg, lmsg_temp);                                        \
                                                                        \
        write_logfile_lock(lmsg);                                       \
    }



#endif //CC_LOG_H
