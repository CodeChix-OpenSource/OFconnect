/*-----------------------------------------------------------------------------*/
/* Copyright: CodeChix Bay Area Chapter 2013                                   */
/*-----------------------------------------------------------------------------*/
#ifndef CC_NET_GLOBAL_H
#define CC_NET_GLOBAL_H

#include <glib.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "cc_net_conn.h"
#include "cc_log.h"

client_svcs_t *client_svcs_map[MAX_L4_TYPE];
server_svcs_t *server_svcs_map[MAX_L4_TYPE];

/*May want to consider hash table instead */
GList *server_list;
GList *client_list;

#endif //CC_NET_GLOBAL_H
