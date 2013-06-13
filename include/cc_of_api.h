/*-----------------------------------------------------------------------------*/
/* Copyright: CodeChix Bay Area Chapter 2013                                   */
/*-----------------------------------------------------------------------------*/
#ifndef CC_OF_API_H
#define CC_OF_API_H

/* TBD: api/callback arguments */
   
/* controller: starts listening on well-known ports */
/* switch: connect to a specific controller */
oflow_create_session()

/* callback to the application when a new controller-switch
 * connection is established */
oflow_new_connection_cb()

/* OF message send */
oflow_send()

/* OF message recv */
oflow_recv()
/* returns immediately if there is nothing in the socket queue */

/* deletes the session */
oflow_delete_session()


#endif
