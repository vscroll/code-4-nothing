/*************************************************************************
	> File Name: vshttpclient_test.c
	> Author: vscroll
	> Mail: vscroll@hotmail.com 
	> Created Time: 2013年12月23日 星期一 21时56分32秒
 ************************************************************************/

#include "vshttpclient.h"

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>
#include <getopt.h>
#define PROG_NAME "vshttpclient_test"

static void usage(void);
static int resp_callback(kVSHttpResp resp, void* resp_data);
int main(int argc, char *argv[])
{
	int c;
	char* url = NULL;
	char* port = NULL;
	char* req = NULL;

	for (;;) {
#define short_options "u:p:r:"

		c = getopt(argc, argv, short_options);
		
		if (c == -1)
			break;
		
		switch (c) {
		case 'u':
			url = optarg;
			break;
		case 'p':
			port = optarg;
			break;
		case 'r':
			req = optarg;
			break;
		default:
			usage();
			break;
		}
	}
	
	if (!url 
	  || !port) { 
	  usage();
	  return -1;
	}
	vshttpclient_init();
	vshttpclient_set_resp_callback(resp_callback);
	
	if (NULL != req)
	{	
		vshttpclient_send_request_test(url, port, req);
	}
	else
	{
	    VSHttpAttackedHostReq http_req;
		memset(&http_req, 0, (size_t)sizeof(http_req));
		http_req.cur_page = -1;
		http_req.ip = 0xFFFFFFFF;
		http_req.port = 0xFFFF;
		http_req.start_time = 0xFFFFFFFF;
		http_req.end_time = 0xFFFFFFFF;
		http_req.ext_ip = 0xFFFFFFFF;
		strcpy(http_req.atk_type, "unknown");
		char out_req_id[REQ_ID_LEN];
		if (vshttpclient_send_request_new(url, port, kAttackedHostQueryReq, &http_req, out_req_id) >= 0)
		{
			vshttp_printf("req_id:%s\n", out_req_id);

			usleep(1000);

			vshttp_printf("send next\n");
			vshttpclient_send_request_next(out_req_id);

			usleep(1000);
			vshttp_printf("send end\n");
			vshttpclient_end_request(out_req_id);

			usleep(1000);
			vshttp_printf("send next\n");
			vshttpclient_send_request_next(out_req_id);	
		}
		
	}
	
	return 0;
}

static void usage(void)
{
  printf ("Usage: " PROG_NAME " [required_options] [optional_options]\n"
          "\n\t[required_options]\n"
          "\t-U --url\t http://192.168.1.1\n"
          "\t-P --port\t 8080\n"
          "\n\t[optional_options]\n"
          "\t-R --request\t \n");
}

 static int resp_callback(kVSHttpResp resp, void* resp_data)
 {
	int i = 0;
	switch (resp)
	{
	case kAttackedHostQueryResp:
		{
			VSHttpAttackedHostPageResp* data = (VSHttpAttackedHostPageResp*)resp_data;
			vshttp_printf("kAttackedHostQueryResp\n");
			vshttp_printf("cur_page:%d\n", data->cur_page);
			vshttp_printf("total_page:%d\n", data->total_page);
			vshttp_printf("cur_num:%d\n", data->cur_num);
			vshttp_printf("req_id:%s\n", data->req_id);
			
			for (i = 0; i < data->cur_num; ++i)
			{
				vshttp_printf("starttime: %u\n", data->resp[i].starttime);
				vshttp_printf("endtime: %u\n", data->resp[i].endtime);
				vshttp_printf("ip_src: %u\n", data->resp[i].ip_src);
				vshttp_printf("ip_dest: %u\n", data->resp[i].ip_dest);
				vshttp_printf("ip_ext: %u\n", data->resp[i].ip_ext);
				vshttp_printf("port_src: %u\n", data->resp[i].port_src);
				vshttp_printf("port_dest: %u\n", data->resp[i].port_dest);	
				vshttp_printf("times: %u\n", data->resp[i].times);
				vshttp_printf("protocal: %u\n", data->resp[i].protocal);	
				vshttp_printf("atk_type: %s\n", data->resp[i].atk_type);		
			}
		
			break;
		}
	default:
		break;
	}
 }