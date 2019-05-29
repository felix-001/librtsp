// Last Update:2019-05-29 20:25:19
/**
 * @file dbg.h
 * @brief 
 * @author felix
 * @version 0.1.00
 * @date 2019-04-30
 */

#ifndef DBG_H
#define DBG_H

#include <stdio.h>

#define BASIC() printf("--- %s %s():%d ---: ", __FILE__, __FUNCTION__, __LINE__ )
#define LOGI(args...) BASIC();printf(args)
#define LOGE(args...) LOGI(args)
#define LogRegular( count, args...) \
    do { \
        static int i =0; \
                         \
        if ( i%count == 0 ) { \
            LOGI(args); \
        } \
        i++; \
    } while(0)

#define DUMPBUF( buf, len ) do { \
    int j =0; \
                \
    printf(#buf" is : \n"); \
    for ( j=0; j<len; j++ ) { \
        printf("%02x ", (unsigned char)(buf)[j]); \
    } \
    printf("\n"); \
} while ( 0 ) 

#endif  /*DBG_H*/
