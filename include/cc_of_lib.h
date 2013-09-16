/*
*****************************************************
**      CodeChix ONF Driver (LibCCOF)
**      codechix.org - May the code be with you...
**              Sept. 15, 2013
*****************************************************
**
** License:        Apache 2.0 (ONF requirement)
** Version:        0.0
** LibraryName:    LibCCOF
** GLIB License:   GNU LGPL
** Description:	   API header for LibCCOF
** Assumptions:    Depends on Glib2.0
** Testing:	   N/A
** Authors:    	   Deepa Karnad Dhurka, Ramya Bolla, Kajal Bhargava
**
*****************************************************
*/

#ifndef CC_OF_LIB_H
#define CC_OF_LIB_H

#define SEND_MSG_BUF_SIZE 1024
char SEND_MSG_BUF[SEND_MSG_BUF_SIZE];

//CC_OF_LIB error codes
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
    CC_OFVER_1_3_1,
    MAX_OFVER_TYPE
} cc_ofver_e;

typedef enum L4_type_ {
    /* used for array index */
    TCP = 0,
    TLS,
    UDP,
    DTLS,
    /* additional types here */ 
    MAX_L4_TYPE
} L4_type_e;

typedef enum of_dev_type_ {
    SWITCH = 0,
    CONTROLLER,
    MAX_OF_DEV_TYPE
} of_dev_type_e;

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
typedef int (*cc_of_recv_pkt)(uint64_t dp_id, uint8_t aux_id,
                              void *of_msg, 
                              size_t of_msg_len);


/**
 * cc_of_accept_channel
 *
 * Description:
 * This callback function is called by the library everytime
 * we accept a new connection. This is to notify the controller 
 * that there is a new connection(of_channel).
 *
 * No-op for switch
 *
 * Returns:
 * Status
 *
 * Notes:
 * 01. This will be a callback. 
 *
 */
typedef int (*cc_of_accept_channel)(uint64_t dummy_dpid,
                                    uint8_t dummy_auxid,
									uint32_t client_ip,
                                    uint16_t client_port);


/**
 * cc_of_delete_channel
 *
 * Description:
 * This callback function is called by the library everytime
 * we a connection is deleted. This is to notify the 
 * controller or switch that a connection is deleted.
 *
 * Returns:
 * Status
 *
 * Notes:
 * 01. This will be a callback. 
 *
 */
typedef int (*cc_of_delete_channel)(uint64_t dpid,
                                    uint8_t auxid);

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
cc_of_ret 
cc_of_lib_init(of_dev_type_e dev_type);

cc_of_ret
cc_of_lib_free(void);

cc_of_ret
cc_of_dev_register(uint32_t controller_ip,
                   uint32_t switch_ip,
                   uint16_t controller_L4_port,
                   cc_ofver_e max_ofver,
                   cc_of_recv_pkt recv_func /*func ptr*/,
                   cc_of_accept_channel accept_func,
                   cc_of_delete_channel del_func);
/* possible additional fields for TLS certificate */

cc_of_ret
cc_of_dev_free(uint32_t controller_ip,
               uint32_t switch_ip,
               uint16_t controller_L4_port);

//CAN BE USED AS STATIC FUNCTION IN CC_OF_LIB.C
cc_of_ret
cc_of_dev_free_lockfree(uint32_t controller_ip,
                        uint32_t switch_ip,
                        uint16_t controller_L4_port);

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
cc_of_ret
cc_of_create_channel(uint32_t controller_ip,
                     uint32_t switch_ip,
                     uint16_t controller_L4_port,
                     uint64_t dp_id, 
                     uint8_t aux_id,
                     L4_type_e l4_proto); /*noop for controller */
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
cc_of_ret 
cc_of_destroy_channel(uint64_t dp_id, 
                      uint8_t aux_id); /*noop for controller */


/**
 * cc_of_send_pkt
 * 
 * Description:
 * This function sends the OF packet to the library to send it to the switch.
 *
 * Returns:
 * Status
 */
cc_of_ret
cc_of_send_pkt(uint64_t dp_id, 
               uint8_t aux_id, 
               void *of_msg, 
               size_t msg_len);


/**
 * cc_of_get_real_dpid_auxid
 * 
 * Description:
 * The controller has to call this function everytime it 
 * establishes a new connection(of_channel). The controller 
 * determines the dp_id/aux_id from the OFP_FEATURES_REQ packet 
 * and notifies the id's to oflib library. Until then oflib 
 * will be using a dummy dp_id/aux_id for the of_channel.
 *
 * No-op for switch
 *
 * Returns:
 * Status
 */
cc_of_ret
cc_of_set_real_dpid_auxid(uint64_t dummy_dpid,
                          uint8_t dummy_auxid,
                          uint64_t dp_id,
                          uint8_t aux_id);


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
cc_of_ret
cc_of_get_conn_stats(uint64_t dp_id, 
                     uint8_t aux_id,
                     uint32_t *rx_pkt,
                     uint32_t *tx_pkt,
                     uint32_t *tx_drops);

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
