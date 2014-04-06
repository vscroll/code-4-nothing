/*************************************************************************
	> File Name: vshttpserver_test.c
	> Author: vscroll
	> Mail: vscroll@hotmail.com 
	> Created Time: 2013年12月23日 星期一 22时13分41秒
 ************************************************************************/

#include "vshttpserver.h"

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>
#include <getopt.h>
#define PROG_NAME "vshttpserver_test"

static void usage(void);
static int query_callback(kVSHttpReq req, void* req_data, void* resp_data);

int main(int argc, char *argv[])
{
	int c;
	char *port = NULL;

	for (;;) {
#define short_options "p:"

		c = getopt(argc, argv, short_options);
		
		if (c == -1)
			break;
		
		switch (c) {
		case 'p':
			port = optarg;
			break;
		default:
			usage();
			break;
		}
	}
	
	if (NULL == port) {
		usage();
		return -1;
	}
	
	vshttp_printf("port:%s\n", port);
	
	vshttpserver_init(port);
	vshttpsever_set_query_callback(query_callback);
	vshttpserver_start();
	
	return 0;
}

static int query_callback(kVSHttpReq req, void* req_data, void* resp_data)
{
	if (NULL == req_data
		||NULL == resp_data)
	{
		return -1;
	}

	switch (req)
	{
	case kAttackedHostQueryReq:
		{
			int i = 0;
			VSHttpAttackedHostPageResp* attacked_host_page_resp = NULL;
			attacked_host_page_resp = (VSHttpAttackedHostPageResp*)resp_data;
			attacked_host_page_resp->cur_num = 10;
			attacked_host_page_resp->cur_page = 0;
			attacked_host_page_resp->total_page = 2;
			
			for (i = 0; i < attacked_host_page_resp->cur_num; ++i)
			{
				attacked_host_page_resp->resp[i].starttime = 0xFFFFFFFFFFFFFFFF-i;
				attacked_host_page_resp->resp[i].endtime = 0xFFFFFFFF-i;
				attacked_host_page_resp->resp[i].ip_src = 0xFFFFFFFF-i;
				attacked_host_page_resp->resp[i].ip_dest = 0xFFFFFFFF-i;
				attacked_host_page_resp->resp[i].ip_ext = 0xFFFF-i;
				attacked_host_page_resp->resp[i].port_src = 0xFFFF-i;
				attacked_host_page_resp->resp[i].port_dest = 0xFFFF-i;
				attacked_host_page_resp->resp[i].times = 0xFFFFFFFF-i;
				attacked_host_page_resp->resp[i].protocal = 0xFFFF-i;
				strcpy(attacked_host_page_resp->resp[i].atk_type, (char*)("unknown"));
			}
			
			break;
		}
	default:
		break;
	}

	return 0;
}

static void usage(void)
{
  printf ("Usage: " PROG_NAME " [required_options] [optional_options]\n"
          "\n\t[required_options]\n"
          "\t-p --port\t 8080,8081 \n"
          "\n\t[optional_options]\n");
}