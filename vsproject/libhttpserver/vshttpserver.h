/*************************************************************************
	> File Name: vshttpserver.h
	> Author: vscroll
	> Mail: vscroll@hotmail.com 
	> Created Time: 2013年12月16日 星期一 23时13分20秒
 ************************************************************************/

#ifndef __VSHTTPSERVER_H_20131216__
#define __VSHTTPSERVER_H_20131216__

#include "../http/vshttp.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int vshttpsever_set_query_callback(vshttpserver_query_callback callback);

int vshttpserver_init(char *port);

int vshttpserver_start();


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
