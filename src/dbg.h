// Last Update:2019-04-30 10:57:12
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

#define BASIC() printf("[ %s %s() +%d ] ", __FILE__, __FUNCTION__, __LINE__ )
#define LOGI(args...) BASIC();printf(args)
#define LOGE(args...) LOGI(args)

#endif  /*DBG_H*/
