#include "HttpRequest.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


HttpRequest::HttpRequest()
{
	InitCurl();
}

HttpRequest::~HttpRequest()
{
	if (NULL != mEasyHandle)
	{
		curl_easy_cleanup(mEasyHandle);
	}
	
	mEasyHandle = NULL;
}

void HttpRequest::SetParseDataCallback(ParseDataCallback Callback)
{
	if (NULL != mEasyHandle)
	{
		curl_easy_setopt(mEasyHandle, CURLOPT_WRITEFUNCTION, Callback);
	}
}

int HttpRequest::SendRequest(const char* HttpUrl, const char* HttpPort, 
	const char* ReqData, const char* ReqId)
{
	int Result = -1;

	if (NULL == mEasyHandle
		|| NULL == HttpUrl
		|| NULL == HttpPort
		|| NULL == ReqData
		|| NULL == ReqId)
	{
		return Result;
	}
	
	char* Url = InitUrl(HttpUrl, HttpPort);

 	curl_easy_setopt(mEasyHandle, CURLOPT_URL, Url);
 	curl_easy_setopt(mEasyHandle, CURLOPT_POSTFIELDS, ReqData); 	
 	curl_easy_setopt(mEasyHandle, CURLOPT_WRITEDATA, ReqId);
	//curl_easy_setopt(mEasyHandle, CURLOPT_WRITEFUNCTION, ParseData);
 	 	
 	Result = curl_easy_perform(mEasyHandle);	
 	if(CURLE_OK != Result)
    {
  		vshttp_printf("%d\n%s\n", Result, "perform failed");
  		Result= -1;
   	}
    
    delete Url;
    Url = NULL;
	
	vshttp_printf("send request new ok\n");
		
	return Result;
}

void HttpRequest::InitCurl()
{
	CURLcode return_code; 
	
 	mEasyHandle = curl_easy_init();
	if (NULL == mEasyHandle)
	{
    	curl_global_cleanup(); 
		return;
	}
	else
	{
		curl_easy_setopt(mEasyHandle, CURLOPT_VERBOSE, 0);
		curl_easy_setopt(mEasyHandle, CURLOPT_TIMEOUT, 5);
		curl_easy_setopt(mEasyHandle, CURLOPT_NOSIGNAL, 1L);
	}

	return;
}

char* HttpRequest::InitUrl(const char* HttpUrl, const char* HttpPort)
{
	const char* Colon = ":";
	const char* QueryHead = "/query";
	int HttpUrlLen = strlen(HttpUrl);
	int HttpPortLen = strlen(HttpPort);
	int ColonLen = strlen(Colon);
	int QueryHeadLen = strlen(QueryHead);
    
	char* Url = NULL;
	int UrlLen = 0;
	int Index = 0;
	
	UrlLen = HttpUrlLen + ColonLen + HttpPortLen + QueryHeadLen;
	Url = new char[UrlLen + 1];
	memset(Url, 0, UrlLen);
	memcpy(Url + Index, HttpUrl, HttpUrlLen);
	Index += HttpUrlLen;
	memcpy(Url + Index, Colon, ColonLen);
	Index += ColonLen;
	memcpy(Url + Index, HttpPort, HttpPortLen);
	Index += HttpPortLen;
	memcpy(Url + Index, QueryHead, QueryHeadLen);

	vshttp_printf("InitUrl:%s\n", Url);
	return Url;
}

