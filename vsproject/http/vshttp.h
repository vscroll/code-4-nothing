#ifndef __VSHTTP_H__
#define __VSHTTP_H__

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef signed int int32;
typedef unsigned int uint32;
typedef unsigned long long uint64;

#define REQ_ID_LEN 16	

#define TIME_LEN_MAX 16
#define IP_LEN_MAX 16
#define PORT_LEN_MAX 8

typedef enum {
        kAttackedHostQueryReq,
        kVSHttpReqMax,
} kVSHttpReq;

typedef enum {
        kAttackedHostQueryResp,
        kVSHttpRespMax,
} kVSHttpResp;

typedef struct {
 	int32 cur_page;
	uint32  start_time;
	uint32  end_time;
	uint32	ext_ip;
	uint32  ip;
	uint16  port;
	char atk_type[16];
} VSHttpAttackedHostReq;

typedef struct {
	uint32  starttime;
	uint32  endtime;
	uint32  ip_src;
	uint32  ip_dest;
	uint32  ip_ext;
	uint16  port_src;
	uint16  port_dest;
	uint32  times;
	uint16  protocal;
	char atk_type[16];
} VSHttpAttackedHostResp;

#define PAGE_LEN_MAX 32
typedef struct {
	int32 cur_page;
	int32 total_page;
	uint16 cur_num;
	char req_id[REQ_ID_LEN];
    VSHttpAttackedHostResp resp[PAGE_LEN_MAX];
} VSHttpAttackedHostPageResp;

typedef int (*vshttpclient_resp_callback)(kVSHttpResp resp, void* resp_data);

typedef int (*vshttpserver_query_callback)(kVSHttpReq req, void* req_data, void* resp_data);

#define DEBUG 1
#if DEBUG
#define vshttp_printf(fmt, ...) printf("%s:%d (%s): " fmt, __FILE__, __LINE__, __func__, ## __VA_ARGS__)
#else
#define vshttp_printf(fmt, ...) 
#endif

#ifdef __cplusplus
}
#endif

#endif
