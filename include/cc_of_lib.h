/*-----------------------------------------------------------------------------*/
/* Copyright: CodeChix Bay Area Chapter 2013                                   */
/*-----------------------------------------------------------------------------*/
#ifndef CC_OF_LIB_H
#define CC_OF_LIB_H

//error codes
#define CC_OF_OK      0
#define CC_OF_EEXIST  -1
#define CC_OF_EINVAL  -2
#define CC_OF_EAGAIN  -3 /* retry */
#define CC_OF_ENOMEM  -4
#define CC_OF_MDEV    -5 /* max out on dev */
#define CC_OF_MCHANN  -6 /* max out on channels */
#define CC_OF_ECHANN  -7 /* unable to establish socket */

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
cc_of_lib_init(of_dev_type_e dev_type, of_drv_type_e drv_type, 
               cc_ofver_e max_ofver_supported);

int
cc_of_lib_free(void);

int
cc_of_dev_register(cc_ofdev_key_t dev_key,
                   uint16_t layer4_port,
                   cc_ofver_e max_ofver,
                   cc_onf_recv_pkt recv_func /*func ptr*/);
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
 * cc_of_recv_pkt
 *
 * Description:
 * This function gets the first packet from the library queue. If the queue is
 * empty, the appropriate error-code will be returned. This function can
 * be called based on a timer. 
 *
 * Returns:
 * Status
 *
 * Notes:
 * 01. This will be a callback. 
 *
 */
typedef (int *cc_onf_recv_pkt)(cc_ofchannel_key_t chann_id,
                               void *of_msg, 
                               uint32_t *of_msg_len);

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
                   uint32_t msg_len);

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


#endif
