// Last Update:2019-05-29 20:49:56
/**
 * @file rtp.c
 * @brief 
 * @author felix
 * @version 0.1.00
 * @date 2019-04-30
 */

#include <sys/socket.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "rtp.h"
#include "dbg.h"
#include "h264_decode.h"

int __rtp_send_h264( int fd, uint8_t *frame, int len, int timestamp )
{
    rtp_pkt_t rtp;
    rtp_hdr_t *p_hdr = &rtp.hdr;
    static unsigned short seq_num = 0;
    ssize_t ret = 0;
    unsigned nal_ref_idc = frame[0] & 0x60;
    unsigned nal_unit_type = frame[0] & 0x1f;
    unsigned char *payload = rtp.payload;

    //LOGI("nal_unit_type = %d, len = %d\n", nal_unit_type, len );
    p_hdr->V = 2;
    p_hdr->P = 0;
    p_hdr->X = 0;
    p_hdr->CC = 0;
    p_hdr->PT = 96;
    p_hdr->seq = htons(seq_num);
    p_hdr->SSRC = 12345;
    p_hdr->timestamp = htonl(timestamp);

    if ( len < __RTP_MAXPAYLOADSIZE ) {
        if ( nal_unit_type != 6 
             && nal_unit_type != 7
             && nal_unit_type != 8 ) {
            p_hdr->M = 1;
        } else {
            p_hdr->M = 0;
        }
        memcpy( &rtp.payload, frame, len );
//        LOGI("p_hdr->seq = %d\n", seq_num );
//        the bug is sizeof(rtp)
//        sizeof(rtp) --> sizeof(rtp.hdr) + len
        ret = send( fd, &rtp, sizeof(rtp.hdr) + len, 0 );
        if ( ret < 0 ) {
            //LOGE("send error\n");
            return -1;
        }
//        usleep( 1000 );
    } else {
        payload[0] = 28;
        payload[0] |= nal_ref_idc;
        payload[1] = nal_unit_type;
        payload[1] |= 1<<7;

        frame += 1;
        len -= 1;
        while( len > __RTP_MAXPAYLOADSIZE - 2 ) {
            p_hdr->seq = htons(seq_num);
            seq_num ++;
            p_hdr->M = 0;
            memcpy( &payload[2], frame, __RTP_MAXPAYLOADSIZE - 2 );
            frame += __RTP_MAXPAYLOADSIZE - 2;
            len -= __RTP_MAXPAYLOADSIZE - 2;
 //           LOGI("p_hdr->seq = %d\n", seq_num );
            ret = send( fd, &rtp, sizeof(rtp), 0 );
            if ( ret < 0 ) {
                //LOGE("send error\n");
                return -1;
            }
//            usleep( 1000 );
            payload[1] &= 0xFF ^ (1<<7); 
        }
        p_hdr->M = 1;
        p_hdr->seq = htons(seq_num);
        payload[1] |= 1 << 6;
        /* intended xor. blame vim :( */
        payload[1] &= 0xFF ^ (1<<7);
        memcpy( &(payload[2]), frame, len );
 //       LOGI("p_hdr->seq = %d\n", seq_num );
        ret = send( fd, &rtp, sizeof(rtp.hdr)+len+2, 0 );
        if ( ret < 0 ) {
            //LOGE("send error\n");
            return -1;
        }
 //       usleep( 1000 );
    }

    seq_num++;
    LogRegular( 500, "already send %d\n", seq_num );
    return 0;
}

int rtp_send_h264( int fd, uint8_t *frame, int len, int timestamp )
{
    NalUnit nalus[10] = { 0 };
    int size = 10;
    int ret = 0;
    int i = 0;

    LOGI("len = 0x%x %d\n", len, len );
    ret = H264ParseNalUnit( (char *)frame, len, nalus, &size );
    if ( ret < 0 ) {
        LOGE("H264ParseNalUnit error\n");
        return -1;
    }
    if ( size <= 0 ) {
        LOGE("get nalus error\n");
        return -1;
    }

    LOGI("size = %d\n", size );
    for ( i=0; i<size; i++ ) {
        LOGI("nalus[%d].size = %d type = %d\n", i, nalus[i].size, nalus[i].type );
        DUMPBUF( nalus[i].addr, 6 );
        DUMPBUF( nalus[i].addr + nalus[i].size  - 6, 6 );
        __rtp_send_h264( fd, (uint8_t *)nalus[i].addr, nalus[i].size, timestamp );
    }
    return 0;
}
