/*-----------------------------------------------------------------------------*/
/* Copyright: CodeChix Bay Area Chapter 2013                                   */
/*-----------------------------------------------------------------------------*/
#ifndef CC_OF_COMMON_H
#define CC_OF_COMMON_H

/* 
 * This header file contains common definations used by library 
 * and also exposed to controller/switch.
 */

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
    CC_OFVER_1_3_1,
    MAX_OFVER_TYPE
} cc_ofver_e;

typedef enum of_dev_type_ {
    SWITCH = 0,
    CONTROLLER,
    MAX_OF_DEV_TYPE
} of_dev_type_e;

typedef enum of_drv_type_ {
    CLIENT = 0,
    SERVER,
    MAX_OF_DRV_TYPE
} of_drv_type_e;

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

#endif
