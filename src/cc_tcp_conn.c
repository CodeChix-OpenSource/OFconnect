#include "cc_of_global.h"
#include "cc_net_conn.h"

// Number of backlog connection requests
#define LISTENQ 1024

/* Forward Declarations */
cc_of_ret tcp_open_clientfd(cc_ofdev_key_t key);
cc_of_ret tcp_open_listenfd(cc_ofdev_key_t key);
cc_of_ret tcp_accept(int listenfd);
cc_of_ret tcp_close(int sockfd);
ssize_t tcp_read(int sockfd, void *buf, size_t len, int flags,
                 struct sockaddr *src_addr UNUSED, 
                 socklen_t *addrlen UNUSED);
ssize_t tcp_write(int sockfd, const void *buf, size_t len, int flags,
                  const struct sockaddr *dest_addr UNUSED,
                  socklen_t addrlen UNUSED);
    

/* Callback Registration for TCP */
net_svcs_t tcp_sockfns = {
    tcp_open_clientfd,
    tcp_open_listenfd,
    tcp_accept,
    tcp_close,
    tcp_read,
    tcp_write,
};

extern net_svcs_t tcp_sockfns;


static void process_sockfd_pollin_func(char *tname UNUSED, void *data UNUSED)
{
    /* TODO: Impl this */
    CC_LOG_ERROR("%s NOT IMPLEMENTED, %s, %p", __FUNCTION__, tname, data);

    /* Allocate buf to read data */     

    /* Read data from socket */

    /* Send data to controller/switch via their callback */
}

static void process_sockfd_pollout_func(char *tname UNUSED, void *data UNUSED)
{
    /* TODO: Impl this */
    CC_LOG_ERROR("%s NOT IMPLEMENTED, %s, %p", __FUNCTION__, tname, data);

    /* Read data from the pipe */

    /* Call socket send fn */

}

cc_of_ret tcp_open_clientfd(cc_ofdev_key_t key)
{
    int clientfd;
    cc_of_ret status = CC_OF_OK;
    int optval = 1;
    struct sockaddr_in serveraddr, localaddr;

    if ((clientfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        CC_LOG_ERROR("%s(%d): %s", __FUNCTION__, __LINE__, strerror(errno));
	    return clientfd;
    }

    // To prevent "Address already in use" error from bind
    if ((status = setsockopt(clientfd, SOL_SOCKET,SO_REUSEADDR, (const void *)&optval, 
                   sizeof(int))) < 0) {
        CC_LOG_ERROR("%s(%d): %s", __FUNCTION__, __LINE__, 
                     strerror(errno));
        return status;
    }
    memset(&localaddr, 0, sizeof(localaddr));
    localaddr.sin_family = AF_INET;
    localaddr.sin_addr.s_addr = ntohl(key.switch_ip_addr);
    localaddr.sin_port = ntohs(key.switch_L4_port);  
    
    // Bind clienfd to a local interface addr
    if ((status = bind(clientfd, (struct sockaddr *) &localaddr, 
                      sizeof(localaddr))) < 0) {
	    CC_LOG_ERROR("%s(%d): %s", __FUNCTION__, __LINE__, strerror(errno));
	    return status;
    }
 
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = ntohl(key.controller_ip_addr);
    serveraddr.sin_port = htons(key.controller_L4_port);

    // Establish connection with server
    if ((status = connect(clientfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr))) < 0) {
	    CC_LOG_ERROR("%s(%d): %s", __FUNCTION__, __LINE__, strerror(errno));
	    return status;
    }
    
    // Add clientfd to a thr_mgr and update it in ofrw, ofdev htbls
    adpoll_thr_msg_t thr_msg;

    thr_msg.fd = clientfd;
    thr_msg.fd_type = SOCKET;
    thr_msg.fd_action = ADD;
    thr_msg.poll_events = POLLIN | POLLOUT;
    thr_msg.pollin_func = &process_sockfd_pollin_func;
    thr_msg.pollin_user_data = NULL; // do we need this ??
    thr_msg.pollout_func = &process_sockfd_pollout_func;
    thr_msg.pollout_user_data = NULL; // do we need this ??
        
    status = cc_add_sockfd_rw_pollthr(&thr_msg, key);
    if (status < 0) {
	    CC_LOG_ERROR("%s(%d):Error updating sockfd in global structures: %s",
                     __FUNCTION__, __LINE__, cc_of_strerror(errno));
	    close(clientfd);
	    return status;
    }

    return clientfd;
}

cc_of_ret tcp_open_listenfd(cc_ofdev_key_t key)
{
    int listenfd;
    cc_of_ret status = CC_OF_OK;
    int optval = 1;
    struct sockaddr_in serveraddr;

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    	CC_LOG_ERROR("%s(%d): %s", __FUNCTION__, __LINE__, strerror(errno));
	    return listenfd;
    }

    // To prevent "Address already in use" error from bind
    if ((status = setsockopt(listenfd, SOL_SOCKET,SO_REUSEADDR, (const void *)&optval, sizeof(int))) < 0) {
	    CC_LOG_ERROR("%s(%d): %s", __FUNCTION__, __LINE__, strerror(errno));
	    return status;
    }

    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = ntohl(key.controller_ip_addr);
    serveraddr.sin_port = htons(key.controller_L4_port);
    
    if ((status = bind(listenfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr))) < 0) {
	    CC_LOG_ERROR("%s(%d): %s", __FUNCTION__, __LINE__, strerror(errno));
	    return status;
    }

    if ((status = listen(listenfd, LISTENQ)) < 0) {
	    CC_LOG_ERROR("%s(%d): %s", __FUNCTION__, __LINE__, strerror(errno));
	    return status;
    }

    return listenfd;
}


cc_of_ret tcp_accept(int listenfd)
{
    int connfd;
    cc_of_ret status;
    struct sockaddr_in clientaddr;
    socklen_t addrlen = sizeof(struct sockaddr_in);
    
    if ((connfd = accept(listenfd, (struct sockaddr *) &clientaddr, &addrlen)) < 0 ) {
	    CC_LOG_ERROR("%s(%d): %s", __FUNCTION__, __LINE__, strerror(errno));
	    return connfd;
    }
    
    // Add connfd to a thr_mgr and update it in ofrw, ofdev htbls
    adpoll_thr_msg_t thr_msg;
    cc_ofdev_key_t key;

    thr_msg.fd = connfd;
    thr_msg.fd_type = SOCKET;
    thr_msg.fd_action = ADD;
    thr_msg.poll_events = POLLIN | POLLOUT;
    thr_msg.pollin_func = &process_sockfd_pollin_func;
    thr_msg.pollin_user_data = NULL; // do we need this ??
    thr_msg.pollout_func = &process_sockfd_pollout_func;
    thr_msg.pollout_user_data = NULL; // do we need this ??
    
    // TODO: Pass dev_key or channel_info?
    /* key.controller_ip_addr = ;
    key.switch_ip_addr = ntohl(clientaddr->sin_addr.s_addr);
    key.switch_L4_port = ntohs(clientaddr->sin_port); */

    status = cc_add_sockfd_rw_pollthr(&thr_msg, key);
    if (status < 0) {
	    CC_LOG_ERROR("%s(%d):Error updating sockfd in global structures: %s",
                     __FUNCTION__, __LINE__, cc_of_strerror(errno));
	    close(connfd);
	    return status;
    }

    return connfd;
}


ssize_t tcp_read(int sockfd, void *buf, size_t len, int flags,
                 struct sockaddr *src_addr UNUSED, 
                 socklen_t *addrlen UNUSED)
{
    return recv(sockfd, buf, len, flags);
} 


ssize_t tcp_write(int sockfd, const void *buf, size_t len, int flags,
                  const struct sockaddr *dest_addr UNUSED,
                  socklen_t addrlen UNUSED)
{
    return send(sockfd, buf, len, flags);
}

/* TODO: Update htbls. Write a 
 * utility fn for del from htbls also 
 */
cc_of_ret tcp_close(int sockfd)
{
    int retval;
    cc_ofrw_key_t rw_key;

    // Update ofrw_htbl
    rw_key.rw_sockfd = sockfd;

    g_mutex_lock(&cc_of_global.ofrw_htbl_lock);
    g_hash_table_remove(cc_of_global.ofrw_htbl, &rw_key);
    g_mutex_unlock(&cc_of_global.ofrw_htbl_lock);

    // Update other two tables??

    if ((retval = close(sockfd)) < 0) {
	CC_LOG_ERROR("%s(%d): %s", __FUNCTION__, __LINE__, cc_of_strerror(errno));
	return -1;
    }

    return retval;
}
