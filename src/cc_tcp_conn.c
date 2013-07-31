#include "cc_of_global.h"

/*
 * Note: This file will not compile right now. I still need to integrate this
 * and make some changes in the data strutures defined in cc_of_global.h 
 * I update this file with comments at places where integration is necessary.
 */


// Number of backlog connection requests
#define LISTENQ 1024

net_svcs_t tcp_sockfns = {
    tcp_open_clientfd,
    tcp_open_listenfd,
    tcp_accept,
    tcp_close,
    tcp_read,
    tcp_write,
};

// Add tcp_sockfns struct to net_svcs_t array in cc_of_global_t??




int tcp_open_clientfd(char *ipaddr, int port)
{
    int clientfd;
    struct sockaddr_in serveraddr;
    cc_ofrw_key_t rw_key;
    cc_ofrw_info_t rw_info;

    if ((clientfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        CC_LOG_ERROR("%s(%d): %s", __FUNCTION__, __LINE__, strerror(errno));
	return -1;
    }

    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(port);
    inet_aton(ipaddr, &serveraddr.sin_addr.s_addr);

    // Establish connection with server
    if (connect(clientfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) {
	CC_LOG_ERROR("%s(%d): %s", __FUNCTION__, __LINE__, strerror(errno));
	return -1;
    }

    
    // Update ofrw_htbl
    rw_key.rw_sockfd = clientfd;    
    rw_info.key = rw_key;
    rw_info.state = CC_OF_RW_UP;
    //rw_info.thr_mgr_p = ?;
    
    g_mutex_lock(&ofrw_htbl_lock);
    g_hash_table_insert(ofrw_htbl, &rw_key, &rw_info);
    g_mutex_unlock(&ofrw_htbl_lock);

    // Update other two hast tables?

    return clientfd;
}


int tcp_open_listenfd(char *ipaddr, int port)
{
    int listenfd;
    int optval = 1;
    struct sockaddr_in serveraddr;

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	CC_LOG_ERROR("%s(%d): %s", __FUNCTION__, __LINE__, strerror(errno));
	return -1;
    }

    // To prevent "Address already in use" error from bind
    if (setsockopt(listenfd, SOL_SOCKET,SO_REUSEADDR, (const void *)&optval, sizeof(int)) < 0) {
	CC_LOG_ERROR("%s(%d): %s", __FUNCTION__, __LINE__, strerror(errno));
	return -1;
    }

    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(port);
    inet_aton(ipaddr, &serveraddr.sin_addr.s_addr);
    
    if (bind(listenfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0) {
	CC_LOG_ERROR("%s(%d): %s", __FUNCTION__, __LINE__, strerror(errno));
	return -1;
    }

    if (listen(listenfd, LISTENQ) < 0) {
	CC_LOG_ERROR("%s(%d): %s", __FUNCTION__, __LINE__, strerror(errno));
	return -1;
    }

    return listenfd;
}


int tcp_accept(int listenfd, struct sockaddr  *clientaddr, int *addrlen)
{
    int connfd;
    cc_ofrw_key_t rw_key;
    cc_ofrw_info_t rw_info;
    
    if ((connfd = accept(listenfd, clientaddr, addrlen)) < 0 ) {
	CC_LOG_ERROR("%s(%d): %s", __FUNCTION__, __LINE__, strerror(errno));
	return -1;
    }
    
    // Update ofrw_htbl
    rw_key.rw_sockfd = connfd;
    rw_info.key = rw_key;
    rw_info.state = CC_OF_RW_UP;
    //rw_info.thr_mgr_p = ?;

    g_mutex_lock(&ofrw_htbl_lock);
    g_hash_table_insert(ofrw_htbl, &rw_key, &rw_info);
    g_mutex_unlock(&ofrw_htbl_lock);

    // Update other two hash tables ??
    
    return connfd;
}


ssize_t tcp_read(int sockfd, void *buf, ssize_t len, int flags) 
{
    return recv(sockfd, buf, len, flags);
} 


ssize_t tcp_write(int sockfd, const void *buf, size_t len, int flags)
{
    return send(sockfd, buf, len, flags);
}


int tcp_close(int sockfd)
{
    int retval;
    cc_ofrw_key_t rw_key;

    // Update ofrw_htbl
    rw_key.rw_sockfd = sockfd;

    g_mutex_lock(&ofrw_htbl_lock);
    g_hash_table_remove(ofrw_htbl, &rw_key);
    g_mutex_unlock(&ofrw_htbl_lock);

    // Update other two tables??

    if ((retval = close(sockfd)) < 0) {
	CC_LOG_ERROR("%s(%d): %s", __FUNCTION__, __LINE__, strerror(errno));
	return -1;
    }

    return retval;
}
