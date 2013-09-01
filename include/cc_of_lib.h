/*-----------------------------------------------------------------------------*/
/* Copyright: CodeChix Bay Area Chapter 2013                                   */
/*-----------------------------------------------------------------------------*/
#ifndef CC_OF_LIB_H
#define CC_OF_LIB_H

#include "cc_net_conn.h"

//error codes
typedef int cc_of_ret;
#define CC_OF_OK        0
#define CC_OF_ESYS     -1  /* syscall, library call error */
#define CC_OF_EINVAL   -2  /* invalid attribute */
#define CC_OF_EAGAIN   -3  /* retry */
#define CC_OF_ENOMEM   -4  /* malloc or other mem max */
#define CC_OF_MDEV     -5  /* max out on dev */
#define CC_OF_EHTBL    -6  /* hash table failures */
#define CC_OF_MCHANN   -7  /* max out on channels */
#define CC_OF_ECHANN   -8  /* unable to establish socket */
#define CC_OF_EEXIST   -9  /* already exists */
#define CC_OF_EMISC    -10 /* misc error */


static const char * cc_of_errtable[] = {
    "okay",
    "syscall/library call failed",
    "invalid attribute",
    "retry",
    "out of memory",
    "max out on devices",
    "hash table failures",
    "max out on channels",
    "unable to establish sockets",
    "already exists",
    "misc error",
};

inline const char *cc_of_strerror(int errnum);

typedef enum cc_ofver_ {
    CC_OFVER_1_0   = 0,
    CC_OFVER_1_3,
    CC_OFVER_1_3_1
} cc_ofver_e;

#define CC_OF_ERRTABLE_SIZE (sizeof(cc_of_errtable) / sizeof(cc_of_errtable[0]))


/**
 * cc_of_recv_pkt
 *
 * Description:
 * This callback function is called by the library when a packet is received
 * from the socket.
 *
 * Returns:
 * Status
 *
 * Notes:
 * 01. This will be a callback. 
 *
 */
typedef int (*cc_of_recv_pkt)(cc_ofchannel_key_t chann_id,
                              void *of_msg, 
                              size_t of_msg_len);

/**
 * cc_of_lib_init
 *
 * Description:
 * This initializes the library.
 *
 * Returns:
 * Status
 *
 * Notes:
 * 01. The library can be initialized in Controller/Switch device type. 
 *
 * 02. The driver mode can be Server/Client.  
 *
 * 03. The MAX_OF_VERSION is the maximum OpenFlow protocol version that 
 *     can be supported by the controller/switch. The library will reject
 *     connections for OpenFlow versions which are not supported.  
 *     
 */
int 
cc_of_lib_init(of_dev_type_e dev_type, of_drv_type_e drv_type);

int
cc_of_lib_free(void);

int 
cc_of_lib_abort(void);


int
cc_of_dev_register(ipaddr_v4v6_t controller_ip_addr,
                   ipaddr_v4v6_t switch_ip_addr,
                   uint16_t controller_L4_port,
                   cc_ofver_e max_ofver,
                   cc_of_recv_pkt recv_func /*func ptr*/);
/* possible additional fields for TLS certificate */

int
cc_of_dev_free(cc_ofdev_key_t dev_key);

/**
 * cc_of_create_channel
 * Note: this api will only be made available to the switch.
 *
 * Description:
 * This function creates the OF connection based on DP_ID and AUX-ID
 *
 * Returns:
 * Status
 *
 */
int 
cc_of_create_channel(cc_ofdev_key_t dev_key,
                     cc_ofchannel_key_t chann_id); /*noop for controller */
/**
 * cc_of_destroy_channel
 *
 * Description:
 * This function destroys the OF connection based on given parameters
 *
 * Returns:
 * Status
 *
 * Notes:
 * 01. Based on the dp-id + aux-id combination of the switch, the OpenFlow
 *     connection will be terminated.
 */
int 
cc_of_destroy_channel(cc_ofchannel_key_t chann_id); /*noop for controller */


/**
 * cc_of_send_pkt
 * 
 * Description:
 * This function sends the OF packet to the library to send it to the switch.
 *
 * Returns:
 * Status
 */
int
cc_of_send_pkt(cc_ofchannel_key_t chann_id, void *of_msg, 
                   size_t msg_len);

/**
 * cc_of_get_conn_stats
 *
 * Description:
 * This function returns the connection stats to the controller/switch based on
 * the dp-id + sw-id.
 *
 * Returns:
 * Status
 */
int
cc_of_get_conn_stats(cc_ofchannel_key_t chann_id,
cc_ofstats_t *stats);

/**
 * cc_of_debug_toggle
 *
 * Description:
 * Enables or disables debug logging
 */
void
cc_of_debug_toggle(gboolean debug_on);

/**
 * cc_of_log_toggle
 *
 * Description:
 * Enables or disables message logging in global log file
 */
void
cc_of_log_toggle(gboolean logging_on);

/**
 * cc_of_log_read
 *
 * Description:
 * Read contents of log file.
 *
 * NOTE: Necessary to g_free the returned pointer
 */

char *
cc_of_log_read();

/**
 * cc_of_log_clear
 *
 * Description:
 * Clear contents of log file.
 *
 */

void
cc_of_log_clear(void);

#endif
