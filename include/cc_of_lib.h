/*-----------------------------------------------------------------------------*/
/* Copyright: CodeChix Bay Area Chapter 2013                                   */
/*-----------------------------------------------------------------------------*/
#ifndef CC_OF_LIB_H
#define CC_OF_LIB_H

#include "cc_of_common.h"

#define SEND_MSG_BUF_SIZE 1024
char SEND_MSG_BUF[SEND_MSG_BUF_SIZE];

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
cc_of_lib_init(of_dev_type_e dev_type, 
               of_drv_type_e drv_type);

int
cc_of_lib_free(void);

int
cc_of_dev_register(uint32_t controller_ip,
                   uint32_t switch_ip,
                   uint16_t controller_L4_port,
                   cc_ofver_e max_ofver,
                   cc_of_recv_pkt recv_func /*func ptr*/);
/* possible additional fields for TLS certificate */

int
cc_of_dev_free(uint32_t controller_ip,
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
int 
cc_of_create_channel(uint32_t controller_ip,
                     uint32_t switch_ip,
                     uint16_t controller_L4_port,
                     uint64_t dp_id, 
                     uint8_t aux_id); /*noop for controller */
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
int
cc_of_send_pkt(uint64_t dp_id, 
               uint8_t aux_id, 
               void *of_msg, 
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
