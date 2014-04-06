#ifndef __HTTP_REQ_BASE_H__
#define __HTTP_REQ_BASE_H__

#include "../http/vshttp.h"

class HttpReqBase {

public:
	HttpReqBase();
	virtual ~HttpReqBase();
	virtual char* GetId() { return mId; };
	virtual int GetSize() = 0;
	virtual int GetType() = 0;
	virtual int SendRequest(const char* HttpUrl, const char* HttpPort) = 0;
	virtual int SendRequestNext() = 0;
	
private:
	void InitId(int len);
	
	char mId[REQ_ID_LEN];
};

#endif