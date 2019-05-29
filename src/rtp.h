// Last Update:2019-05-14 11:30:27
/**
 * @file rtp.h
 * @brief 
 * @author felix
 * @version 0.1.00
 * @date 2019-05-13
 */

#ifndef RTP_H
#define RTP_H

#define __RTP_MAXPAYLOADSIZE 1460

typedef struct {
#ifdef _BIG_ENDIAN
    unsigned V:2;
    unsigned P:1;
    unsigned X:1;
    unsigned CC:4;
    unsigned M:1;
    unsigned PT:7;
#else
    unsigned CC:4;
    unsigned X:1;
    unsigned P:1;
    unsigned V:2;
    unsigned PT:7;
    unsigned M:1;
#endif
    unsigned seq:16;
    uint32_t timestamp;
    uint32_t SSRC;
} rtp_hdr_t;

typedef struct {
    rtp_hdr_t hdr;
    unsigned char payload[__RTP_MAXPAYLOADSIZE];
} rtp_pkt_t;

extern int rtp_send_h264( int fd, uint8_t *frame, int len, int timestamp );

#endif  /*RTP_H*/
