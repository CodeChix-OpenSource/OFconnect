#include "cc_of_global.h"
#include "cc_of_priv.h"

/* Forward Declarations */
int udp_open_clientfd(cc_ofdev_key_t key, cc_ofchannel_key_t ofchann_key);
int udp_open_serverfd(cc_ofdev_key_t key);
int udp_close(int sockfd);
ssize_t udp_read(int sockfd, void *buf, size_t len, int flags,
	             struct sockaddr *src_addr, socklen_t *addrlen);
ssize_t udp_write(int sockfd, const void *buf, size_t len, int flags,
	              const struct sockaddr *dest_addr, socklen_t addrlen);

net_svcs_t udp_sockfns = {
    udp_open_clientfd,
    udp_open_serverfd,
    NULL,
    udp_close,
    udp_read,
    udp_write,
};


static void process_udpfd_pollin_func(char *tname UNUSED,
                                      adpoll_fd_info_t *data_p,
                                      adpoll_send_msg_htbl_info_t *unused_data UNUSED)
{
    char buf[MAXBUF]; /* Allocate buf to read data */
    ssize_t read_len = 0;
    cc_ofchannel_key_t *fd_chann_key;
    int udp_sockfd = data_p->fd;
    cc_of_ret status = CC_OF_OK;
    cc_ofrw_key_t rwkey;
    cc_ofrw_info_t *rwinfo = NULL;
    cc_ofdev_info_t *devinfo = NULL;
    struct sockaddr_in src_addr;
    socklen_t addrlen;
    
    if (data_p == NULL) {
        CC_LOG_ERROR("%s(%d): received NULL data",
                     __FUNCTION__, __LINE__);
        return;
    }

    /* Read data from socket */
    if ((read_len = udp_read(udp_sockfd, buf, MAXBUF, 0, 
                             (struct sockaddr *)&src_addr, &addrlen)) < 0) {
        CC_LOG_ERROR("%s(%d): %s, Error while reading pkt on udp sockfd: %d",
                     __FUNCTION__, __LINE__, strerror(errno), udp_sockfd);
        return;
    }

    CC_LOG_INFO("%s(%d):, Read pkt on udp sockfd: %d"
               "from srcIP-%s,srcPort-%u", __FUNCTION__, __LINE__, 
               udp_sockfd, inet_ntoa(src_addr.sin_addr), ntohs(src_addr.sin_port));

    status = find_ofchann_key_rwsocket(udp_sockfd, &fd_chann_key);
    if (status < 0) {
        CC_LOG_ERROR("%s(%d): could not find ofchann key for sockfd %d",
                     __FUNCTION__, __LINE__, udp_sockfd);
        return;
    }

    /* 
     * Do a reverse lookup to get the dev_key 
     * corresponding to this udp sockfd
     */
    rwkey.rw_sockfd = udp_sockfd;
    rwinfo = g_hash_table_lookup(cc_of_global.ofrw_htbl, &rwkey);
    if (rwinfo == NULL) {
        CC_LOG_ERROR("%s(%d): could not find rwsockinfo in ofrw_htbl"
                     "for sockfd-%d", __FUNCTION__, __LINE__, rwkey.rw_sockfd);
        return;
    }

    devinfo = g_hash_table_lookup(cc_of_global.ofdev_htbl, &(rwinfo->dev_key));
    if (devinfo == NULL) {
        CC_LOG_ERROR("%s(%d): could not find devinfo in ofdev_htbl"
                     "for device", __FUNCTION__, __LINE__);
        return;
    }

    /* Send data to controller/switch via their callback */
    devinfo->recv_func(fd_chann_key->dp_id, fd_chann_key->aux_id, 
                        buf, read_len);
    CC_LOG_INFO("%s(%d): read a pkt on udp sockfd: %d, aux_id: %lu, dp_id: %u"
                "and sent it to controller/switch", __FUNCTION__, __LINE__, 
                udp_sockfd, fd_chann_key->dp_id, fd_chann_key->aux_id);
}


static void process_udpfd_pollout_func(char *tname UNUSED,
                                       adpoll_fd_info_t *data_p,
                                       adpoll_send_msg_htbl_info_t *send_msg_p)
{
    int udp_sockfd = 0;
    struct sockaddr_in dest_addr;
    socklen_t addrlen = sizeof(dest_addr);

    if (data_p == NULL) {
        CC_LOG_ERROR("%s(%d): received NULL data",
                     __FUNCTION__, __LINE__);
    }

    if (send_msg_p == NULL) {
        CC_LOG_ERROR("%s(%d): send message invalid",
                     __FUNCTION__, __LINE__);
    }

    udp_sockfd = data_p->fd;
    if (getpeername(udp_sockfd,(struct sockaddr *)&dest_addr, &addrlen) < 0) {
        CC_LOG_ERROR("%s(%d): %s, error while getting peername for udp sockfd: %d",
                      __FUNCTION__, __LINE__, strerror(errno), udp_sockfd);
        return;
    }

    /* Call udpsocket send fn */
    if (udp_write(udp_sockfd, send_msg_p->data, send_msg_p->data_size, 
                  0, (struct sockaddr *)&dest_addr, addrlen) < 0) {
        CC_LOG_ERROR("%s(%d): %s, error while sending pkt on udp sockfd: %d", 
                     __FUNCTION__, __LINE__, strerror(errno), udp_sockfd);
        return;
    } 

    CC_LOG_INFO("%s(%d): sent a pkt out on udp sockfd: %d", __FUNCTION__, 
                __LINE__, udp_sockfd);

}


int udp_open_clientfd(cc_ofdev_key_t key, cc_ofchannel_key_t ofchann_key)
{
    int clientfd;
    cc_of_ret status = CC_OF_OK;
    int optval = 1;
    struct sockaddr_in serveraddr, localaddr;

    if ((clientfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        CC_LOG_ERROR("%s(%d): %s", __FUNCTION__, __LINE__, strerror(errno));
	    return clientfd;
    }

    /* Aux connections shoud use the same srcIP as main connection.
     * Hence, bind clientfd to srcIP.
     */

    // To prevent "Address already in use" error from bind
    if ((status = setsockopt(clientfd, SOL_SOCKET,SO_REUSEADDR, 
                             (const void *)&optval, sizeof(int))) < 0) {
        CC_LOG_ERROR("%s(%d): %s", __FUNCTION__, __LINE__, strerror(errno));
        return status;
    }
    memset(&localaddr, 0, sizeof(localaddr));
    localaddr.sin_family = AF_INET;
    localaddr.sin_addr.s_addr = htonl(key.switch_ip_addr);
    localaddr.sin_port = 0;  
    
    // Bind clienfd to a local interface addr
    if ((status = bind(clientfd, (struct sockaddr *) &localaddr, 
                       sizeof(localaddr))) < 0) {
	    CC_LOG_ERROR("%s(%d): %s", __FUNCTION__, __LINE__, strerror(errno));
	    return status;
    }

    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(key.controller_ip_addr);
    serveraddr.sin_port = htons(key.controller_L4_port);

    // Establish connection with server
    if ((status = connect(clientfd, (struct sockaddr *) &serveraddr, 
                          sizeof(serveraddr))) < 0) {
	    CC_LOG_ERROR("%s(%d): %s", __FUNCTION__, __LINE__, strerror(errno));
	    return status;
    }
    
    // Add clientfd to a thr_mgr and update it in ofrw, ofdev htbls
    adpoll_thr_msg_t thr_msg;

    thr_msg.fd = clientfd;
    thr_msg.fd_type = SOCKET;
    thr_msg.fd_action = ADD;
    thr_msg.poll_events = POLLIN | POLLOUT;
    thr_msg.pollin_func = &process_udpfd_pollin_func;
    thr_msg.pollout_func = &process_udpfd_pollout_func;
        
    status = cc_add_sockfd_rw_pollthr(&thr_msg, key, TCP, ofchann_key);
    if (status < 0) {
	    CC_LOG_ERROR("%s(%d):Error updating udp sockfd in global structures: %s",
                     __FUNCTION__, __LINE__, cc_of_strerror(errno));
	    close(clientfd);
	    return status;
    }

    return clientfd;

}


int udp_open_serverfd(cc_ofdev_key_t key)
{
    int serverfd;
    int optval = 1;
    struct sockaddr_in serveraddr;
    adpoll_thr_msg_t thr_msg;
    cc_ofchannel_key_t ofchann_key;

    if ((serverfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
	    CC_LOG_ERROR("%s(%d): %s", __FUNCTION__, __LINE__, cc_of_strerror(errno));
	    return -1;
    }

    // To prevent "Address already in use" error from bind
    if (setsockopt(serverfd, SOL_SOCKET,SO_REUSEADDR, (const void *)&optval, sizeof(int)) < 0) {
	    CC_LOG_ERROR("%s(%d): %s", __FUNCTION__, __LINE__, cc_of_strerror(errno));
	    return -1;
    }

    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(key.controller_L4_port);
    serveraddr.sin_addr.s_addr = htonl(key.controller_ip_addr);
    
    if (bind(serverfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0) {
	    CC_LOG_ERROR("%s(%d): %s", __FUNCTION__, __LINE__, cc_of_strerror(errno));
	    return -1;
    }

    // Add udpfd to a pollthrmgr
    thr_msg.fd = serverfd;
    thr_msg.fd_type = SOCKET;
    thr_msg.fd_action = ADD;
    thr_msg.poll_events = POLLIN|POLLOUT;
    thr_msg.pollin_func = &process_udpfd_pollin_func;
    thr_msg.pollout_func = &process_udpfd_pollout_func;

    cc_add_sockfd_rw_pollthr(&thr_msg, key, UDP, ofchann_key);        

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
    cc_of_ret status = CC_OF_OK;
    adpoll_thread_mgr_t *tmgr = NULL;
    adpoll_thr_msg_t thr_msg;
    
    thr_msg.fd = sockfd;
    thr_msg.fd_type = SOCKET;

    status = find_thrmgr_rwsocket(sockfd, &tmgr);
    if (status < 0) {
        CC_LOG_ERROR("%s(%d): could not find tmgr for udp sockfd %d",
                     __FUNCTION__, __LINE__, sockfd);
        return status;
    }

    // Update global htbls
    status = cc_del_sockfd_rw_pollthr(tmgr, &thr_msg);
    if (status < 0) {
        CC_LOG_ERROR("%s(%d): %s, error while deleting udp sockfd %d "
                     "from global structures", __FUNCTION__, __LINE__, 
                     cc_of_strerror(status), sockfd);
        return status;
    }

    if ((status = close(sockfd)) < 0) {
	    CC_LOG_ERROR("%s(%d): %s", __FUNCTION__, __LINE__, strerror(errno));
	    return status;
    }

    return status;
}
