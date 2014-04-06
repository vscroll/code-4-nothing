#include "AttackedHostHttpReq.h"

#include <string.h>
#include <libxml/tree.h>

static vshttpclient_resp_callback mRespCallback = NULL;

AttackedHostHttpReq::AttackedHostHttpReq(VSHttpAttackedHostReq*  ReqData)
{
	memcpy(&mReqData, ReqData, sizeof(mReqData));
	mReqData.cur_page = -1;
	mHttpRequest.SetParseDataCallback(ParseData);

	mHttpUrl = NULL;
	mHttpPort = NULL;	
}

AttackedHostHttpReq::~AttackedHostHttpReq()
{
	if (NULL != mHttpUrl)
	{
		delete mHttpUrl;
		mHttpUrl = NULL;
	}

	if (NULL != mHttpPort)
	{
		delete mHttpPort;
		mHttpPort = NULL;
	}

}

void AttackedHostHttpReq::SetRespCallback(vshttpclient_resp_callback Callback)
{
	mRespCallback = Callback;
}

int AttackedHostHttpReq::SendRequest(const char* HttpUrl, const char* HttpPort)
{
	if (NULL == mHttpUrl)
	{
		mHttpUrl = new char[strlen(HttpUrl) + 1];
		strcpy(mHttpUrl, HttpUrl);
	}

	if (NULL == mHttpPort)
	{
		mHttpPort = new char[strlen(HttpPort) + 1];
		strcpy(mHttpPort, HttpPort);
	}

	char* ReqData = BuildReqData();
	int Result = mHttpRequest.SendRequest(HttpUrl, HttpPort, ReqData, GetId());

	delete ReqData;
	ReqData = NULL;

	return Result;
}

int AttackedHostHttpReq::SendRequestNext()
{
	if (NULL == mHttpUrl
		|| NULL == mHttpPort)
	{
		return -1;
	}

	mReqData.cur_page++;
	
	
	return SendRequest(mHttpUrl, mHttpPort);
}

/*
<?xml version="1.0"?>
<attacked_host_query>
  <cur_page>0</cur_page>
  <start_time>00:00</start_time>
  <end_time>23:59</end_time>
  <ip>192.168.58.132</ip>
  <port>8080</port>
  <ext_ip>192.168.58.132</ext_ip>
  <atk_type>unknown</atk_type>
</attacked_host_query>
*/
char* AttackedHostHttpReq::BuildReqData()
{
	char* p = NULL;
	xmlChar *xml_buff = NULL;
	int buffer_size = 0;
	char tmp[32] = {0};

	xmlDocPtr doc = xmlNewDoc(BAD_CAST"1.0");
	xmlNodePtr root_node = xmlNewNode(NULL,BAD_CAST"attacked_host_query");
	xmlDocSetRootElement(doc, root_node);
	
	memset(tmp, 0, sizeof(tmp));
	sprintf(tmp, "%d", mReqData.cur_page);
	xmlNewTextChild(root_node, NULL, BAD_CAST"cur_page", BAD_CAST tmp);
	
	memset(tmp, 0, sizeof(tmp));
	sprintf(tmp, "%u", mReqData.start_time);
	xmlNewTextChild(root_node, NULL, BAD_CAST"start_time", BAD_CAST tmp);

	memset(tmp, 0, sizeof(tmp));
	sprintf(tmp, "%u", mReqData.end_time);	
	xmlNewTextChild(root_node, NULL, BAD_CAST"end_time", BAD_CAST tmp);

	memset(tmp, 0, sizeof(tmp));
	sprintf(tmp, "%u", mReqData.ip);
	xmlNewTextChild(root_node, NULL, BAD_CAST"ip", BAD_CAST tmp);
	
	memset(tmp, 0, sizeof(tmp));
	sprintf(tmp, "%u", mReqData.port);	
	xmlNewTextChild(root_node, NULL, BAD_CAST"port", BAD_CAST tmp);
	
	memset(tmp, 0, sizeof(tmp));
	sprintf(tmp, "%u", mReqData.ext_ip);	
	xmlNewTextChild(root_node, NULL, BAD_CAST"ext_ip", BAD_CAST tmp);
	
	xmlNewTextChild(root_node, NULL, BAD_CAST"atk_type", BAD_CAST mReqData.atk_type);

	xmlDocDumpFormatMemory(doc, &xml_buff, &buffer_size, 1);
	//xmlDocDumpMemory(doc, &xml_buff, &buffer_size);

	p = new char[buffer_size + 1];
	memset(p, 0, buffer_size + 1);
	memcpy(p, (char*)xml_buff, buffer_size);
	xmlFree(xml_buff);
	xmlFreeDoc(doc);
	doc = NULL;
	xmlCleanupParser();
	xmlMemoryDump();
	vshttp_printf("%s\n", p);

	return p;
}

size_t AttackedHostHttpReq::ParseData(char *Buffer, size_t Size, size_t Nmemb, void *UserData)
{
	vshttp_printf(" buffer:%s\n size:%zu\n reqid:%s", Buffer, Size*Nmemb, (char*)UserData);

	int len = strlen(Buffer);
	if (strcmp(Buffer, "null"))
	{
		VSHttpAttackedHostPageResp RespData;
		
		//xmlDocPtr doc = xmlParseMemory(Buffer, Size);
		xmlDocPtr doc = xmlReadMemory(Buffer, len, NULL, NULL, 0);
		if (NULL == doc)
		{
			vshttp_printf("parse_resp_data doc is null\n");
			return Size*Nmemb;
		}
		
		xmlNodePtr root = xmlDocGetRootElement(doc);
		if (NULL == root)
		{
			vshttp_printf("parse_resp_data root is null\n");
			xmlFreeDoc(doc);
			return Size*Nmemb;
		}

		if (!xmlStrcmp(root->name, BAD_CAST"attacked_host_page_resp"))
		{
			int count = -1;
			xmlNodePtr child_node = root->xmlChildrenNode;
			while (child_node != NULL)
			{
				if (!xmlStrcmp(child_node->name, BAD_CAST"cur_page"))
				{
					xmlChar *szKey = xmlNodeGetContent(child_node);
					vshttp_printf("cur_page: %s\n", szKey);
					RespData.cur_page = atoi((char*)szKey);
					xmlFree(szKey);
				}
				else if (!xmlStrcmp(child_node->name, BAD_CAST"total_page"))
				{
					xmlChar *szKey = xmlNodeGetContent(child_node);
					vshttp_printf("total_page: %s\n", szKey);
					RespData.total_page = atoi((char*)szKey);
					xmlFree(szKey);
				}
				else if (!xmlStrcmp(child_node->name, BAD_CAST"cur_num"))
				{
					xmlChar *szKey = xmlNodeGetContent(child_node);
					vshttp_printf("cur_num: %s\n", szKey);
					RespData.cur_num = atoi((char*)szKey);
					xmlFree(szKey);
				}
				else if (!xmlStrcmp(child_node->name, BAD_CAST"attacked_host_resp"))
				{
					count++;
					vshttp_printf("attacked_host_resp:%d\n", count);
					ParseChileNode(child_node->children, count, &RespData);
				}
				else
				{
				
				}
			
				child_node = child_node->next;
			}
		}	
		
		memcpy(RespData.req_id, UserData, REQ_ID_LEN);
		mRespCallback(kAttackedHostQueryResp, &RespData);
		
		xmlFreeDoc(doc);
		doc = NULL;
		xmlCleanupParser();
		xmlMemoryDump();
	}

	return Size*Nmemb;
}

void AttackedHostHttpReq::ParseChileNode(void* Node, int Index, VSHttpAttackedHostPageResp* Resp)
{
    xmlNode* CurNode = NULL;
 
    for (CurNode = (xmlNode*)Node; CurNode; CurNode = CurNode->next) {
	
		if (!xmlStrcmp(CurNode->name, BAD_CAST"starttime"))
		{
			xmlChar *szKey = xmlNodeGetContent(CurNode);
			vshttp_printf("starttime: %s\n", szKey);
			Resp->resp[Index].starttime = atoi((char*)szKey);
			xmlFree(szKey);		
		}
		else if(!xmlStrcmp(CurNode->name, BAD_CAST"endtime"))
		{
			xmlChar *szKey = xmlNodeGetContent(CurNode);
			vshttp_printf("endtime: %s\n", szKey);
			Resp->resp[Index].endtime = atoi((char*)szKey);
			xmlFree(szKey);	
		}
		else if(!xmlStrcmp(CurNode->name, BAD_CAST"ip_ext"))
		{
			xmlChar *szKey = xmlNodeGetContent(CurNode);
			vshttp_printf("ip_ext: %s\n", szKey);
			Resp->resp[Index].ip_ext = atoi((char*)szKey);
			xmlFree(szKey);	
		}
		else if(!xmlStrcmp(CurNode->name, BAD_CAST"ip_src"))
		{
			xmlChar *szKey = xmlNodeGetContent(CurNode);
			vshttp_printf("ip_src: %s\n", szKey);
			Resp->resp[Index].ip_src = atoi((char*)szKey);
			xmlFree(szKey);	
		}
		else if(!xmlStrcmp(CurNode->name, BAD_CAST"ip_dest"))
		{
			xmlChar *szKey = xmlNodeGetContent(CurNode);
			vshttp_printf("ip_dest: %s\n", szKey);
			Resp->resp[Index].ip_dest = atoi((char*)szKey);
			xmlFree(szKey);	
		}
		else if(!xmlStrcmp(CurNode->name, BAD_CAST"port_src"))
		{
			xmlChar *szKey = xmlNodeGetContent(CurNode);
			vshttp_printf("port_src: %s\n", szKey);
			Resp->resp[Index].port_src = atoi((char*)szKey);
			xmlFree(szKey);	
		}
		else if(!xmlStrcmp(CurNode->name, BAD_CAST"port_dest"))
		{
			xmlChar *szKey = xmlNodeGetContent(CurNode);
			vshttp_printf("port_dest: %s\n", szKey);
			Resp->resp[Index].port_dest = atoi((char*)szKey);
			xmlFree(szKey);	
		}
		else if(!xmlStrcmp(CurNode->name, BAD_CAST"times"))
		{
			xmlChar *szKey = xmlNodeGetContent(CurNode);
			vshttp_printf("times: %s\n", szKey);
			Resp->resp[Index].times = atoi((char*)szKey);
			xmlFree(szKey);	
		}
		else if(!xmlStrcmp(CurNode->name, BAD_CAST"protocal"))
		{
			xmlChar *szKey = xmlNodeGetContent(CurNode);
			vshttp_printf("protocal: %s\n", szKey);
			Resp->resp[Index].protocal = atoi((char*)szKey);
			xmlFree(szKey);	
		}
		else if(!xmlStrcmp(CurNode->name, BAD_CAST"atk_type"))
		{
			xmlChar *szKey = xmlNodeGetContent(CurNode);
			vshttp_printf("atk_type: %s\n", szKey);
			strcpy(Resp->resp[Index].atk_type,(char*)(szKey));
			xmlFree(szKey);	
		}
		else
		{
		
		}
    }
}
