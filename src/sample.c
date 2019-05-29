// Last Update:2019-05-28 17:10:36
/**
 * @file sample.c
 * @brief  ipc push rtsp stream, could use ffplay to play
 * @author felix
 * @version 0.1.00
 * @date 2019-04-30
 */

#include <unistd.h>
#include <stdio.h>
#include "dbg.h"
#include "rtsp.h"
#include "ipc.h"
#include "rtp.h"

typedef struct {
    rtsp_context_t *ctx;
} app_t;

static app_t app;

int VideoFrameCallBack ( uint8_t *frame, int len, int iskey, int64_t timestamp )
{
    if ( app.ctx->start_play ) {
        rtp_send_h264( app.ctx->rtp_fd, frame, len, timestamp );
    }

    return 0;
}

int AudioFrameCallBack ( uint8_t *frame, int len, int64_t timestamp )
{
    return 0;
}

int main()
{
    rtsp_param_t param = 
    {
        .port = 554
    };
    ipc_param_t ipc_param =
    {
        .audio_type = AUDIO_AAC,
        .video_file = "./video.h264",
        .audio_file = NULL,
        .pic_file = NULL,
        .video_cb = VideoFrameCallBack,
        .audio_cb = AudioFrameCallBack,
        .event_cb = NULL,
        .log_output = NULL 
    };

    app.ctx = rtsp_new_context( &param );
    app.ctx->start_play = 0;

    LOGI("enter main\n");

    ipc_init( &ipc_param );
    while( !app.ctx->start_play ) {
        sleep( 3 );
    }
    LOGI("ipc_run\n");
    ipc_run();

    for (;;) {
        sleep( 3 );
    }

    return 0;
}

