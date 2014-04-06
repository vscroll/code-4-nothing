/*************************************************************************
	> File Name: HttpRequest.h
	> Author: vscroll
	> Mail: vscroll@hotmail.com 
	> Created Time: 2014年03月08日 星期六 09时58分55秒
 ************************************************************************/

#ifndef __HTTP_REQUEST_H__
#define __HTTP_REQUEST_H__

#include <curl/curl.h>
#include "../http/vshttp.h"

typedef size_t (*ParseDataCallback)(char *Buffer, size_t Size, size_t Nmemb, void *UserData);

class HttpRequest {

public:
	HttpRequest();
	virtual ~HttpRequest();
	void SetParseDataCallback(ParseDataCallback Callback);
	int SendRequest(const char* HttpUrl, const char* HttpPort,
		const char* ReqData, const char* ReqId);

private:	
	void InitCurl();
	char* InitUrl(const char* HttpUrl, const char* HttpPort);
	
	CURL* mEasyHandle;
};



#endif
