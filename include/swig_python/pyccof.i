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
** Description:    SWIG Python interface file for LibCCOF
** Assumptions:    python 2.7/2.6, swig 2.0.10
** Testing:        N/A
** Authors:        Swapna Iyer
**
*****************************************************
*/

%module pyccof 

//Make pyccof_wrap.c include this header
%{

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


typedef enum L4_type_ {
    /* used for array index */
    TCP = 0,
    TLS,
    UDP,
    DTLS,
    /* additional types here */
    MAX_L4_TYPE
} L4_type_e;
                      

cc_of_ret
cc_of_create_channel(uint32_t controller_ip,
                     uint32_t switch_ip,
                     uint16_t controller_L4_port,
                     uint64_t dp_id,
                     uint8_t aux_id,
                     L4_type_e l4_proto); /*noop for controller */
cc_of_ret
cc_of_destroy_channel(uint64_t dp_id,
                      uint8_t aux_id); /*noop for controller */


cc_of_ret
cc_of_send_pkt(uint64_t dp_id,
               uint8_t aux_id,
               void *of_msg,
               size_t msg_len);


%}

//Make swig look into specific stuff in this header

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


typedef enum L4_type_ {
    /* used for array index */
    TCP = 0,
    TLS,
    UDP,
    DTLS,
    /* additional types here */
    MAX_L4_TYPE
} L4_type_e;
                      


cc_of_ret
cc_of_create_channel(uint32_t controller_ip,
                     uint32_t switch_ip,
                     uint16_t controller_L4_port,
                     uint64_t dp_id,
                     uint8_t aux_id,
                     L4_type_e l4_proto); /*noop for controller */
cc_of_ret
cc_of_destroy_channel(uint64_t dp_id,
                      uint8_t aux_id); /*noop for controller */


cc_of_ret
cc_of_send_pkt(uint64_t dp_id,
               uint8_t aux_id,
               void *of_msg,
               size_t msg_len);





