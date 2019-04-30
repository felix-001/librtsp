// Last Update:2019-04-30 11:08:19
/**
 * @file public.h
 * @brief 
 * @author felix
 * @version 0.1.00
 * @date 2019-04-30
 */

#ifndef PUBLIC_H
#define PUBLIC_H

#define CHECK_PARAM( param ) \
    if ( !param ) { \
        LOGE("check %s error\n", #param ); \
        return NULL; \
    }

#define ARRSZ(arr) sizeof(arr)/sizeof(arr[0])

#endif  /*PUBLIC_H*/
