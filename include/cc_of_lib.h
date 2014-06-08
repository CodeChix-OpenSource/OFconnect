/*
****************************************************************
**      CodeChix OFconnect - OpenFlow Channel Management Library
**      Copyright CodeChix 2013-2014
**      codechix.org - May the code be with you...
****************************************************************
**
** License:             GPL v2
** Version:             0.0
** Project/Library:     OFconnect/libccof.so
** GLIB License:        GNU LGPL
** Description:         API Declarations for OFconnect
** Authors:    	        Deepa Karnad Dhurka
**                      Ramya Bolla
**                      Kajal Bhargava
**
** Main Contact:        deepa.dhurka@gmail.com
** Alt. Contact:        organizers@codechix.org
****************************************************************
*/

#ifndef CC_OF_LIB_H
#define CC_OF_LIB_H

#define SEND_MSG_BUF_SIZE 1024
char SEND_MSG_BUF[SEND_MSG_BUF_SIZE];

/* CC_OF_LIB error codes */
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

/* String definitions for error codes */
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

/*
 *****************************************
 *          LIBRARY CALLBACKS            *
 *****************************************
 */
/**
 * cc_of_accept_channel
 *
 * Description:
 * Callback Function
 * Called any time the library accepts a new connection.
 * Notifies the device (controller) of a new connection.
 *
 * Not supported for switch device type
 *
 * Arguments:
 * dummy_dpid: DPID assigned by library before real DPID is learnt
 * dummy_auxid: AUXID assigned by library before real AUXID is learnt
 * client_ip: IP address of the peering switch
 * client_port: L4 port of the peering switch
 *
 * Returns:
 * Status
 */
typedef int (*cc_of_accept_channel)(uint64_t dummy_dpid,
                                    uint8_t dummy_auxid,
                                    uint32_t client_ip,
                                    uint16_t client_port);


/**
 * cc_of_delete_channel
 *
 * Description:
 * Callback Function
 * Called any time the library deletes a connection.
 *
 * Arguments:
 * dpid: DPID of channel to be deleted
 * auxid: AUXID of channel to be deleted
 *
 * Returns:
 * Status
 */
typedef int (*cc_of_delete_channel)(uint64_t dpid,
                                    uint8_t auxid);


/**
 * cc_of_recv_pkt
 *
 * Description:
 * Callback Function
 * Called any time a packet is received from the socket.
 *
 * Arguments:
 * dp_id: DPID of the channel on which pkt was rx
 * aux_id: AUXID of the channel on which pkt was rx
 * of_msg: pointer to OF packet
 * of_msg_len: size of OF packet in bytes
 *
 * Returns:
 * Status
 */
typedef int (*cc_of_recv_pkt)(uint64_t dp_id, uint8_t aux_id,
                              void *of_msg, 
                              size_t of_msg_len);



/*
 *****************************************
 *             LIBRARY API               *
 *****************************************
 */
/**
 * cc_of_lib_init
 *
 * Description:
 * Initializes the library.
 *
 * Arguments:
 * dev_type: specify if the calling SDN appliance is controller
 *           or switch
 *
 * Returns:
 * Status
 */
cc_of_ret 
cc_of_lib_init(of_dev_type_e dev_type);


/**
 * cc_of_lib_free
 *
 * Description:
 * Frees the library.
 *
 * Returns:
 * Status
 */
cc_of_ret
cc_of_lib_free(void);


/**
 * cc_of_dev_register
 *
 * Description:
 * If device type is controller:
 *     Register a specific combination of controller IP + L4 port
 * If device type is switch:
 *     Register a specific combination of controller (IP + L4 port)
 *         and switch IP
 * The library calls this unique combination a 'device'
 * If controller is active on multiple IPs and ports, register once
 *    for each combination.
 * If switch is active on multiple IPs or connecting to multiple
 *    controllers, register once for each such combination.
 *
 * Arguments:
 * controller_ip: IPv4 address of controller. 
 * switch_ip: IPv4 address of switch.
 *            Used only if calling device is a switch.
 * max_ofver: max OpenFlow version supported by device
 *            Unused field.
 * recv_func: callback function registration
 *            (see cc_of_recv_pkt for info)
 * accept_func: callback function registration
 *              (see cc_of_accept_channel for info)
 * del_func: callback function registration
 *           (see cc_of_delete_channel for info)
 *
 * Returns:
 * Status
 *
 * Notes:
 * May need enhancement to support TLS
 */
cc_of_ret
cc_of_dev_register(uint32_t controller_ip,
                   uint32_t switch_ip,
                   uint16_t controller_L4_port,
                   cc_ofver_e max_ofver,
                   cc_of_recv_pkt recv_func,
                   cc_of_accept_channel accept_func,
                   cc_of_delete_channel del_func);


/**
 * cc_of_dev_free
 *
 * Description:
 * This function releases the specific device registration
 *
 * Arguments:
 * controller_ip: IPv4 address of controller
 * switch_ip: IPv4 address of switch (used only when device is switch)
 * controller_L4_port: Transport port number of controller
 *
 * Returns:
 * Status
 *
 */
cc_of_ret
cc_of_dev_free(uint32_t controller_ip,
               uint32_t switch_ip,
               uint16_t controller_L4_port);

/**
 * cc_of_create_channel
 *
 * Description:
 * This function creates the OF connection based on DP_ID and AUX-ID
 * The library supports this only for the switch device type
 *
 * Arguments:
 * controller_ip: IPv4 of controller to connect to
 * switch_ip: IPv4 of switch to connect from
 * controller_L4_port: layer 4 port number of controller
 * dp_id: switch-generated DPID, used in OF messages
 * aux_id: switch-generated AUXID, used in OF messages
 * l4_proto: Transport protocol for the connection
 *           Used only by the switch device type
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
                     L4_type_e l4_proto);


/**
 * cc_of_destroy_channel
 *
 * Description:
 * Brings down the OF connection
 * Supported only for switch device type
 *
 * Returns:
 * Status
 */
cc_of_ret 
cc_of_destroy_channel(uint64_t dp_id, 
                      uint8_t aux_id);


/**
 * cc_of_send_pkt
 * 
 * Description:
 * Send the OF packet to the library to send it to the peering SDN device
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
 * cc_of_set_real_dpid_auxid
 * 
 * Description:
 * The controller has to call this function everytime it 
 * establishes a new connection(of_channel). The controller 
 * determines the dp_id/aux_id from the OFP_FEATURES_REQ packet 
 * and notifies the id's to oflib library. Until then the library
 * uses a generated dummy dp_id/aux_id for the of_channel.
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
 * Query of connection statistics
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
 * A new log file is generated any time this is called
 *     with logging_on TRUE
 * Path: $HOME/.OFconnect/log-<timestamp>
 */
void
cc_of_log_toggle(gboolean logging_on);

/**
 * cc_of_log_read
 *
 * Description:
 * Return contents of log file as a character stream
 *
 * NOTE: Necessary to g_free the returned pointer
 */
char *
cc_of_log_read();

/**
 * cc_of_log_clear
 *
 * Description:
 * Clear contents of global log file.
 *
 */
void
cc_of_log_clear(void);

#endif
