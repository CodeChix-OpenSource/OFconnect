#include <glib.h>
#include "cc_net_conn.h"

client_svcs_t tcp_client = {
    tcp_client_open,
    tcp_client_close,
    tcp_client_connect,
    tcp_client_recv,
    tcp_client_send
};

server_svcs_t tcp_server = {
    tcp_server_open,
    tcp_server_close,
    tcp_server_listen,
    tcp_server_accept,
    NULL,
    NULL
};

int
tcp_client_open() {



}

void
tcp_client_close() {


}

int
tcp_client_connect() {


}

int
tcp_client_recv() {


}

int
tcp_client_send() {


}
