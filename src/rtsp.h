// Last Update:2019-04-30 11:49:07
/**
 * @file rtsp.h
 * @brief 
 * @author felix
 * @version 0.1.00
 * @date 2019-04-30
 */

#ifndef RTSP_H
#define RTSP_H


typedef struct {
    int sockfd;
    int connfd;
}rtsp_context_t;

typedef struct {
    char *url;
    int port;
} rtsp_param_t;

extern rtsp_context_t * rtsp_new_context( rtsp_param_t *param );

#endif  /*RTSP_H*/
