// Last Update:2019-04-30 14:26:05
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

typedef struct {
    rtsp_context_t *ctx;
} app_t;

static app_t app;

int main()
{
    rtsp_param_t param = 
    {
        .port = 554
    };

    app.ctx = rtsp_new_context( &param );

    LOGI("enter main\n");

    for (;;) {
        sleep( 3 );
    }

    return 0;
}

