/*
#include <glib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
//#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
*/
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
    tcp_read,
    tcp_write,
    tcp_close
};

// Add tcp_sockfns struct to net_svcs_t array in cc_of_global_t



int tcp_open_clientfd(char *ipaddr, int port)
{
    int clientfd;
    struct sockaddr_in serveraddr;

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

    // Update GHTable with the new fd ??

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
    
    if ((connfd = accept(listenfd, clientaddr, addrlen)) < 0 ) {
	CC_LOG_ERROR("%s(%d): %s", __FUNCTION__, __LINE__, strerror(errno));
	return -1;
    }
    
    // Update GHTable with the new fd ??

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

    if ((retval = close(sockfd)) < 0) {
	CC_LOG_ERROR("%s(%d): %s", __FUNCTION__, __LINE__, strerror(errno));
	return -1;
    }

    // Update GHTable to remove the sockfd ??

    return retval;
}
