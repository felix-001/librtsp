// Last Update:2019-05-08 20:45:35
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
    RTSP_SETUP,
    RTSP_PLAY,
};

static rtsp_cmd_t cmd_list[] = 
{
    { "OPTIONS", RTSP_OPTIONS },
    { "DESCRIBE", RTSP_DESCRIBE },
    { "SETUP", RTSP_SETUP },
    { "PLAY", RTSP_PLAY }
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

int rtsp_handle_play( rtsp_context_t *ctx )
{
    int ret = 0;
    char *resp = "RTSP/1.0 200 OK\r\n"
        "CSep: 4\r\n"
        "Session: 313720730\r\n\r\n";
    int fd = 0, opt = 1;
    struct sockaddr_in addr;

    ret = write( ctx->connfd, resp, strlen(resp) );
    if ( ret <= 0 ) {
        LOGE("send response error\n");
        return -1;
    }

    fd = socket( AF_INET, SOCK_DGRAM, 0 );
    if ( fd < 0 ) {
        LOGE("create socket error\n");
        return -1;
    }

    addr.sin_port = htons( 10036 );
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_family = AF_INET;
    setsockopt( fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt) );

    ret = bind( fd, (struct sockaddr *)&addr, sizeof(addr) );
    if ( ret < 0 ) {
        LOGE("bind error\n");
        return -1;
    }
    addr = ctx->client_addr;
    addr.sin_port = htons( ctx->client_port_rtp );

    ret = connect( fd, (struct sockaddr *)&addr, sizeof(addr) );
    if ( ret < 0 ) {
        LOGE("connect error\n");
        return -1;
    }
    return 0;
}

int rtsp_handle_setup( rtsp_context_t *ctx, char *buf )
{
    int ret = 0;
    char *resp = "RTSP/1.0 200 OK\r\n"
        "CSeq: 3\r\n"
        "Session: 313720730\r\n"
        "Transport: RTP/AVP/UDP;unicast;client_port=11489-11490\r\n\r\n";
    char *s = "client_port=";
    char *start = strstr( buf, s );

    if ( !start ) {
        LOGE("parse transport error\n");
        return -1;
    }

    start += strlen(s);
    ret = sscanf( start, "%u-%u", &ctx->client_port_rtp, &ctx->client_port_rtcp );

    LOGI("rtp port: %d, rtcp port : %d\n", ctx->client_port_rtp, ctx->client_port_rtcp );

    ret = write( ctx->connfd, resp, strlen(resp) );
    if ( ret <= 0 ) {
        LOGE("send response error\n");
        return -1;
    }
    return 0;
}

int rtsp_handle_describe( rtsp_context_t *ctx )
{
    int ret = 0;
    char *sdp = "v=0\r\n"
                "o=- 0 0 IN IP4 127.0.0.1\r\n"
                "s=librtsp\r\n"
                "c=IN IP4 0.0.0.0\r\n"
                "t=0 0\r\n"
                "a=tool:libavformat 52.73.0\r\n"
                "m=video 0 RTP/AVP 96\r\n"
                "a=rtpmap:96 H264/90000\r\n"
                "a=fmtp:96 packetization-mode=1\r\n"
                "a=control:streamid=0\r\n"; 

    char resp[512] = { 0 };

    sprintf( resp, 
            "RTSP/1.0 200 OK\r\n"
            "CSeq: 2\r\n"
            "Content-Type: application/sdp\r\n"
            "Content-Length: %ld\r\n"
            "\r\n"
            "%s" 
            , strlen(sdp), sdp 
             );

    ret = write( ctx->connfd, resp, strlen(resp) );
    if ( ret <= 0 ) {
        LOGE("send response error\n");
        return -1;
    }
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
    case RTSP_SETUP:
        rtsp_handle_setup( ctx, buf );
        break;
    case RTSP_PLAY:
        rtsp_handle_play( ctx );
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
        ctx->client_addr = cli_addr;
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

