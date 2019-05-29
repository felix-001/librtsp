// Last Update:2019-05-08 20:43:15
/**
 * @file rtsp.h
 * @brief 
 * @author felix
 * @version 0.1.00
 * @date 2019-04-30
 */

#ifndef RTSP_H
#define RTSP_H

#include <netinet/in.h>
#include <stdlib.h>
#include <arpa/inet.h>

typedef struct {
    int sockfd;
    int connfd;
    int client_port_rtp;
    int client_port_rtcp;
    int rtp_fd;
    struct sockaddr_in client_addr;
    int start_play;
}rtsp_context_t;

typedef struct {
    char *url;
    int port;
} rtsp_param_t;

extern rtsp_context_t * rtsp_new_context( rtsp_param_t *param );

#endif  /*RTSP_H*/
