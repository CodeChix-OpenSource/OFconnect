#include "cc_of_global.h"

/*
 * Note: This file will not compile right now. I still need to integrate this
 * and make some changes in the data strutures defined in cc_of_global.h 
 * I update this file with comments at places where integration is necessary.
 */

net_svcs_t udp_sockfns = {
    udp_open_clientfd,
    udp_open_serverfd,
    NULL,
    udp_close
    udp_read,
    udp_write,
};

// Add tcp_sockfns struct to net_svcs_t array in cc_of_global_t



int udp_open_clientfd(char *ipaddr, int port)
{
    int clientfd;
    struct sockaddr_in serveraddr;

    if ((clientfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        CC_LOG_ERROR("%s(%d): %s", __FUNCTION__, __LINE__, strerror(errno));
	return -1;
    }

    // Update GHTable with the new fd ??

    return clientfd;
}


int udp_open_serverfd(char *ipaddr, int port)
{
    int serverfd;
    int optval = 1;
    struct sockaddr_in serveraddr;

    if ((serverfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
	CC_LOG_ERROR("%s(%d): %s", __FUNCTION__, __LINE__, strerror(errno));
	return -1;
    }

    // To prevent "Address already in use" error from bind
    if (setsockopt(serverfd, SOL_SOCKET,SO_REUSEADDR, (const void *)&optval, sizeof(int)) < 0) {
	CC_LOG_ERROR("%s(%d): %s", __FUNCTION__, __LINE__, strerror(errno));
	return -1;
    }

    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(port);
    inet_aton(ipaddr, &serveraddr.sin_addr.s_addr);
    
    if (bind(serverfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0) {
	CC_LOG_ERROR("%s(%d): %s", __FUNCTION__, __LINE__, strerror(errno));
	return -1;
    }

    return serverfd;
}


ssize_t udp_read(int sockfd, void *buf, size_t len, int flags,
	         struct sockaddr *src_addr, socklen_t *addrlen)
{
    return recvfrom(sockfd, buf, len, flags, src_addr, addrlen);
} 


ssize_t udp_write(int sockfd, const void *buf, size_t len, int flags,
	          const struct sockaddr *dest_addr, socklen_t addrlen)
{
    return sendto(sockfd, buf, len, flags, dest_addr, addrlen);
}


int udp_close(int sockfd)
{
    int retval;

    if ((retval = close(sockfd)) < 0) {
	CC_LOG_ERROR("%s(%d): %s", __FUNCTION__, __LINE__, strerror(errno));
	return -1;
    }

    // Update GHTable to remove the sockfd ??

    return retval;
}
