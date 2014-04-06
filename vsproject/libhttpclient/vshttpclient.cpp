/*************************************************************************
	> File Name: vshttpclient.c
	> Author: vscroll
	> Mail: vscroll@hotmail.com 
	> Created Time: 2013年12月22日 星期日 22时39分30秒
 ************************************************************************/

#include "vshttpclient.h"
#include<stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <libxml/tree.h>

#include "HttpReqBase.h"
#include "AttackedHostHttpReq.h"

vshttpclient_resp_callback g_resp_callback = NULL;

#define CACHE_COUNT_MAX 1024
static HttpReqBase* gHttpReq[CACHE_COUNT_MAX] = {NULL};
static int g_write_index = 0;

static HttpReqBase* find_request(char* req_id);
static void save_request(HttpReqBase* http_req);
static void update_request(HttpReqBase* http_req);
static void delete_request(char* req_id);

int vshttpclient_init()
{
	CURLcode return_code; 

	return_code = curl_global_init(CURL_GLOBAL_ALL); 	
	if (CURLE_OK != return_code) 	
	{ 		
		vshttp_printf("%s\n","init libcurl failed.");
		return -1; 
	}
	
	return 0;
}

int vshttpclient_uninit()
{
	curl_global_cleanup();
}

void vshttpclient_set_resp_callback(vshttpclient_resp_callback callback)
{
	g_resp_callback = callback;
}

int vshttpclient_send_request_new(const char* http_url, const char* http_port,
		kVSHttpReq req, void* req_data, char out_req_id[])
{
	int result = -1;
	
	if (NULL == http_url
	    || NULL == http_port
	    || NULL == req_data
	    || NULL == out_req_id)
	{
	    vshttp_printf("param err\n");
	    return result;
	}
	
	memset(out_req_id, 0, REQ_ID_LEN);
	
	switch (req)
	{
	case kAttackedHostQueryReq:
		{
			AttackedHostHttpReq* AttackedHostReq = 
				new AttackedHostHttpReq((VSHttpAttackedHostReq*)req_data);
			AttackedHostReq->SetRespCallback(g_resp_callback);
			strcpy(out_req_id, AttackedHostReq->GetId());
			result = AttackedHostReq->SendRequest(http_url, http_port);
			if (result >= 0)
			{
				save_request(AttackedHostReq);
			}
			break;
		}
	default:
		break;
	}

	return result;
}

int vshttpclient_send_request_next(char* req_id)
{
	HttpReqBase* p = NULL;

	if (NULL == req_id)
	{
		return -1;
	}
	
	p = find_request(req_id);
	if (NULL != p)
	{
		p->SendRequestNext();
	}
	else
	{
		vshttp_printf("request is not exist\n");
	}
	return 0;
}

int vshttpclient_end_request(char* req_id)
{
	if (NULL != req_id)
	{
		delete_request(req_id);
	}

	return 0;
}

void vshttpclient_send_request_test(const char* http_url,
    const char* http_port, const char* req)
{
       //char out_req_id[REQ_ID_LEN];
	//send_request(http_url, http_port, req, out_req_id);
	return;
}

static HttpReqBase* find_request(char* req_id)
{
	int i = 0;
	HttpReqBase* p = NULL;
	if (NULL == req_id)
	{
		return p;
	}
	
	for (i = 0; i < CACHE_COUNT_MAX; i++)
	{
		if (NULL != gHttpReq[i])
		{
			if (!strcmp(gHttpReq[i]->GetId(), req_id))
			{
				p = gHttpReq[i];
				break;
			}
		}
	}

	return p;
}

static void save_request(HttpReqBase* http_req)
{
	HttpReqBase* p = NULL;

	if (NULL == http_req)
	{
		return;
	}
	
	p = find_request(http_req->GetId());
	if (NULL == p)
	{
		vshttp_printf("save request:%s\n", http_req->GetId());
		gHttpReq[g_write_index] = http_req;
		g_write_index = g_write_index + 1;
	}
}

static void update_request(HttpReqBase* http_req)
{
	HttpReqBase* p = NULL;

	if (NULL == http_req)
	{
		return;
	}
	
	p = find_request(http_req->GetId());
	if (NULL != p)
	{
		memcpy(p, http_req, http_req->GetSize());
	}
}

static void delete_request(char* req_id)
{
	int i = 0;
	HttpReqBase* p = NULL;
	
	if (NULL == req_id)
	{
		return;
	}

	for (i = 0; i < CACHE_COUNT_MAX; i++)
	{
		if (NULL != gHttpReq[i])
		{
			if (!strcmp(gHttpReq[i]->GetId(), req_id))
			{
				delete gHttpReq[i];
				gHttpReq[i] = NULL;
				break;
			}
		}
	}
}

