/*-----------------------------------------------------------------------------*/
/* Copyright: CodeChix Bay Area Chapter 2013                                   */
/*-----------------------------------------------------------------------------*/
#ifndef CC_LOG_H
#define CC_LOG_H

#include <glib.h>
#include <glib/gprintf.h>

#define CC_LOG_DOMAIN    ("cc_onf_driver_lib")

#define CC_LOG_ENABLE_DEBUGS()                     \
    {                                              \
        g_setenv("G_MESSAGES_DEBUG", "all", TRUE); \
    }

#define CC_LOG_DISABLE_DEBUGS()                    \
    {                                              \
        g_unsetenv("G_MESSAGES_DEBUG");            \
    }

#define CC_LOG_WARNING(...)\
    g_log(CC_LOG_DOMAIN, G_LOG_LEVEL_WARNING, __VA_ARGS__)

#define CC_LOG_DEBUG(...)\
    g_log(CC_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, __VA_ARGS__)

#define CC_LOG_CRITICAL(...)\
    g_log(CC_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, __VA_ARGS__)

#define CC_LOG_INFO(...)\
    g_log(CC_LOG_DOMAIN, G_LOG_LEVEL_INFO, __VA_ARGS__)

#define CC_LOG_ERROR(...)\
    g_log(CC_LOG_DOMAIN, G_LOG_LEVEL_ERROR, __VA_ARGS__)

#endif //CC_LOG_H
