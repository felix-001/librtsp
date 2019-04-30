// Last Update:2019-04-30 16:46:18
/**
 * @file rtsp.c
 * @brief 
 * @author felix
 * @version 0.1.00
 * @date 2019-04-30
 */

#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "dbg.h"
#include "rtsp.h"
#include "public.h"

typedef struct {
    char *str;
    int cmd; 
} rtsp_cmd_t;

enum {
    RTSP_OPTIONS,
    RTSP_DESCRIBE,
};

static rtsp_cmd_t cmd_list[] = 
{
    { "OPTIONS", RTSP_OPTIONS },
    { "DESCRIBE", RTSP_DESCRIBE }
};

int rtsp_get_cmd( char *buf )
{
    int i = 0;
    
    for ( i=0; i<ARRSZ( cmd_list ); i++ ) {
        if ( strncmp( cmd_list[i].str, buf, strlen(cmd_list[i].str) ) == 0 ) {
            return cmd_list[i].cmd;
        }
    }

    return -1;
}

int rtsp_handle_options( rtsp_context_t *ctx )
{
    int ret = 0;
    char *resp = "RTSP/1.0 200 OK\r\n"
                "CSeq: 1\r\n"
                "Public: OPTIONS, DESCRIBE, PLAY, PAUSE, SETUP, TEARDOWN, SET_PARAMETER, GET_PARAMETER\r\n"
                "Date: Tue, Apr 30 2019 16:08:47 GMT\r\n\r\n";
    /*
     * if the end of the string have no \r\n
     * will not receive DESCRIBE
     * */

    ret = write( ctx->connfd, resp, strlen(resp) );
    if ( ret <= 0 ) {
        LOGE("send response error\n");
        return -1;
    }

    return 0;
}

int rtsp_handle_describe( rtsp_context_t *ctx )
{
    char *resp = "RTSP/1.0 200 OK\r\n"
        "CSeq: 4\r\n"
        "Content-Type: application/sdp\r\n"
        "Content-Base: rtsp://172.17.11.155:554/h264/ch1/main/av_stream/\r\n"
        "Content-Length: 598\r\n\r\n"
    return 0;
}

int rtsp_handle( rtsp_context_t *ctx, char *buf, int len  )
{
    int cmd = 0;

    LOGI("buf = %s\n", buf );

    cmd = rtsp_get_cmd( buf );
    if ( cmd == -1 ) {
        LOGE("get cmd error\n");
        return -1;
    }

    switch( cmd ) {
    case RTSP_OPTIONS:
        rtsp_handle_options( ctx );
        break;
    case RTSP_DESCRIBE:
        rtsp_handle_describe( ctx );
        break;
    }

    return 0;
}

void *rtsp_eventloop( void *arg )
{
    rtsp_context_t *ctx = (rtsp_context_t *)arg;
    char buf[1024] = { 0 };
    int n = 0, ret = 0;

    CHECK_PARAM( ctx );

    for (;;) {
        LOGI("start to read\n");
        n = read( ctx->connfd, buf, sizeof(buf) );
        LOGI("n = %d\n", n );
        if ( n <= 0 ) {
            LOGE("read error\n");
            return NULL;
        }
        ret = rtsp_handle( ctx, buf, strlen(buf) );
        if ( ret < 0 ) {
            LOGE("rtsp_handle error\n");
            return NULL;
        }
    }

    return NULL;
}

void *rtsp_monitoring_task( void *arg )
{
    rtsp_context_t *ctx = (rtsp_context_t *)arg;
    struct sockaddr_in cli_addr;
    socklen_t clilen;
    pthread_t thread;

    CHECK_PARAM( ctx );

    clilen = sizeof( cli_addr);
    for (;;) {
        ctx->connfd = accept( ctx->sockfd, (struct sockaddr *) &cli_addr, &clilen );
        if( ctx->connfd < 0 ) {
            LOGE("accept error\n");
            return NULL;
        }
        LOGI("new connection form %s : %d\n", inet_ntoa(cli_addr.sin_addr), cli_addr.sin_port );
        pthread_create( &thread, NULL, rtsp_eventloop, ctx );
    }
    return NULL;
}

rtsp_context_t * rtsp_new_context( rtsp_param_t *param )
{
    rtsp_context_t *ctx = NULL;
    int v = 1, ret = 0;
    struct sockaddr_in s;
    pthread_t thread = 0;

    CHECK_PARAM( param );

    ctx = ( rtsp_context_t *) malloc( sizeof(rtsp_context_t) );
    if ( !ctx ) {
        LOGE("malloc error\n");
        goto err;
    }
    
    ctx->sockfd = socket( AF_INET, SOCK_STREAM, 0 );
    if ( ctx->sockfd < 0 ) {
        LOGE("create socket error\n");
        goto err;
    }
    setsockopt( ctx->sockfd, SOL_SOCKET, SO_REUSEADDR, (void *)&v, sizeof(int) );

    s.sin_family = AF_INET;
    s.sin_addr.s_addr = htonl( INADDR_ANY);
    s.sin_port = htons(param->port);
    ret = bind( ctx->sockfd, (struct sockaddr *) &s, sizeof(s) );
    if ( ret < 0 ) {
        LOGE("bind error\n");
        goto err;
    }

    ret = listen( ctx->sockfd, 5 );
    if ( ret < 0 ) {
        LOGE("listen error\n");
        goto err;
    }

    pthread_create( &thread, NULL, rtsp_monitoring_task, ctx );
    
    return ctx;

err:
    if ( ctx )
        free(ctx);
    return NULL;
}

