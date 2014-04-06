/*************************************************************************
	> File Name: HttpRequest.h
	> Author: vscroll
	> Mail: vscroll@hotmail.com 
	> Created Time: 2014年03月08日 星期六 09时58分55秒
 ************************************************************************/

#ifndef __ATTACKED_HOST_HTTP_REQ_H__
#define __ATTACKED_HOST_HTTP_REQ_H__

#include "HttpReqBase.h"
#include "HttpRequest.h"

class AttackedHostHttpReq:public HttpReqBase {

public:
	AttackedHostHttpReq(VSHttpAttackedHostReq*  ReqData);
	virtual ~AttackedHostHttpReq();
	virtual int GetSize() { return sizeof(AttackedHostHttpReq); };
	virtual int GetType() { return kAttackedHostQueryReq; };
	void SetRespCallback(vshttpclient_resp_callback Callback);
	virtual int SendRequest(const char* HttpUrl, const char* HttpPort);
	virtual int SendRequestNext();
private:
	char* BuildReqData();
	static size_t ParseData(char *Buffer, size_t Size, size_t Nmemb, void *UserData);
	static void ParseChileNode(void* Node, int Index, VSHttpAttackedHostPageResp* Resp);
	
	VSHttpAttackedHostReq mReqData;
	HttpRequest mHttpRequest;
	char* mHttpUrl;
	char* mHttpPort;
};

#endif
