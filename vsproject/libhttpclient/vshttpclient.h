/*************************************************************************
	> File Name: vshttpclient.h
	> Author: vscroll
	> Mail: vscroll@hotmail.com 
	> Created Time: 2013年12月22日 星期日 22时05分39秒
 ************************************************************************/

#ifndef __VSHTTPCLIENT_H__
#define __VSHTTPCLIENT_H__

#include "../http/vshttp.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int vshttpclient_init();

int vshttpclient_uninit();

void vshttpclient_set_resp_callback(vshttpclient_resp_callback callback);

int vshttpclient_send_request_new(const char* http_url, const char* http_port,
		kVSHttpReq req, void* req_data, char out_req_id[]);

int vshttpclient_send_request_next(char* req_id);

int vshttpclient_end_request(char* req_id);
		
void vshttpclient_send_request_test(const char* http_url, const char* http_port,
		const char* req);

#ifdef __cplusplus
}
#endif

#endif
