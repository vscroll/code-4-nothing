/*************************************************************************
	> File Name: vshttpserver.c
	> Author: vscroll
	> Mail: vscroll@hotmail.com 
	> Created Time: 2013年12月16日 星期一 23时15分43秒
 ************************************************************************/

#include "vshttpserver.h"

#include <sys/types.h>
#include <sys/select.h>
#include <sys/wait.h>
#define ALIAS_URI "/index.htm"
#define ALIAS_DIR "./webroot"

#include <time.h>
#include <errno.h>
#include <signal.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "shttpd.h"
#include "../http/vshttp.h"
#include <libxml/tree.h>

static struct shttpd_ctx *g_ctx = NULL;

static void signal_handler(int sig_num);
static void show_index(struct shttpd_arg *arg);
static void show_404(struct shttpd_arg *arg);
static void show_secret(struct shttpd_arg *arg);
static void show_query(struct shttpd_arg *arg);
static void resp_200(struct shttpd_arg *arg);
static void resp_500(struct shttpd_arg *arg);
static void resp_null_package(struct shttpd_arg *arg);
static void resp_one_package(struct shttpd_arg *arg, 
	kVSHttpReq req, void* resp_data);
static int parse_post_data(char* post_data, int post_data_len, kVSHttpReq* req, void** req_data);
static char* build_attacked_host_resp_data(VSHttpAttackedHostPageResp* resp_data);

static vshttpserver_query_callback g_query_callback = NULL;

int vshttpserver_init(char *port)
{
	int			data = 1234567;
	struct shttpd_ctx	*ctx;
	

	signal(SIGPIPE, SIG_IGN);
	signal(SIGCHLD, &signal_handler);

	/*
	 * Initialize SHTTPD context.
	 * Attach folder c:\ to the URL /my_c  (for windows), and
	 * /etc/ to URL /my_etc (for UNIX). These are Apache-like aliases.
	 * Set WWW root to current directory.
	 * Start listening on ports 8080 and 8081
	 */
	g_ctx = ctx = shttpd_init(0, NULL);
	//shttpd_set_option(ctx, "ssl_cert", "shttpd.pem");
	shttpd_set_option(ctx, "aliases", ALIAS_URI "=" ALIAS_DIR);
	//shttpd_set_option(ctx, "ports", "8080,8081s");
	shttpd_set_option(ctx, "ports", port);

	/* Register an index page under two URIs */
	shttpd_register_uri(ctx, "/", &show_index, (void *) &data);
	
	shttpd_register_uri(ctx, "/query", &show_query, (void *) &data);

	/* Show how to use password protection */
	shttpd_register_uri(ctx, "/secret", &show_secret, NULL);
	shttpd_set_option(ctx, "protect", "/secret=passfile");

	shttpd_handle_error(ctx, 404, show_404, NULL);
	
	return 0;
}

int vshttpserver_start()
{
	struct shttpd_ctx *ctx = g_ctx;
	if (NULL == ctx)
	{
		return -1;
	}
	
	/* Serve connections infinitely until someone kills us */
	for (;;)
		shttpd_poll(ctx, 1000);

	/* Probably unreached, because we will be killed by a signal */
	shttpd_fini(ctx);

	return 0;
}

int vshttpsever_set_query_callback(vshttpserver_query_callback callback)
{
	g_query_callback = callback;
	
	return 0;
}

static void signal_handler(int sig_num)
{
	switch (sig_num) {
	case SIGCHLD:
		while (waitpid(-1, &sig_num, WNOHANG) > 0) ;
		break;
	default:
		break;
	}
}

static void show_index(struct shttpd_arg *arg)
{
	int		*p = (int*)arg->user_data;	/* integer passed to us */
	char		value[20];
	const char	*host, *request_method, *query_string, *request_uri;

	request_method = shttpd_get_env(arg, "REQUEST_METHOD");
	request_uri = shttpd_get_env(arg, "REQUEST_URI");
	query_string = shttpd_get_env(arg, "QUERY_STRING");


	/* Change the value of integer variable */
	value[0] = '\0';
	if (!strcmp(request_method, "POST")) {
		/* If not all data is POSTed, wait for the rest */
		if (arg->flags & SHTTPD_MORE_POST_DATA)
			return;
		(void) shttpd_get_var("name1", arg->in.buf, arg->in.len,
		    value, sizeof(value));
	} else if (query_string != NULL) {
		(void) shttpd_get_var("name1", query_string,
		    strlen(query_string), value, sizeof(value));
	}
	
	if (value[0] != '\0') {
		*p = atoi(value);

		/*
		 * Suggested by Luke Dunstan. When POST is used,
		 * send 303 code to force the browser to re-request the
		 * page using GET method. This prevents the possibility of
		 * the user accidentally resubmitting the form when using
		 * Refresh or Back commands in the browser.
		 */
		if (!strcmp(request_method, "POST")) {
			shttpd_printf(arg, "HTTP/1.1 303 See Other\r\n"
				"Location: %s\r\n\r\n", request_uri);
			arg->flags |= SHTTPD_END_OF_OUTPUT;
			return;
		}
	}
	
	shttpd_printf(arg, "%s",
		"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"
		"<html><body><h1>Welcome to embedded example </h1><ul>");
	//shttpd_printf(arg, " v. %s </h1><ul>", shttpd_version());

	shttpd_printf(arg, "<li><code>REQUEST_METHOD: %s "
		"REQUEST_URI: \"%s\" QUERY_STRING: \"%s\""
		" REMOTE_ADDR: %s REMOTE_USER: \"(null)\"</code><hr>",
		request_method, request_uri,
		query_string ? query_string : "(null)",
		shttpd_get_env(arg, "REMOTE_ADDR"));
	shttpd_printf(arg, "<li>Internal int variable value: <b>%d</b>", *p);

	shttpd_printf(arg, "%s",
		"<form method=\"POST\">Enter new value: "
		"<input type=\"text\" name=\"name1\"/>"
		"<input type=\"submit\" "
		"value=\"set new value using POST method\"></form>");
		
	shttpd_printf(arg, "%s",
		"<hr><li><a href=\"/secret\">"
		"Protected page</a> (guest:guest)<hr>"
		"<li><a href=\"" ALIAS_URI "/\">Aliased " ALIAS_DIR " directory</a><hr>");

	host = shttpd_get_header(arg, "Host");
	shttpd_printf(arg, "<li>'Host' header value: [%s]<hr>",
		host ? host : "NOT SET");

	shttpd_printf(arg, "%s", "</body></html>");
	arg->flags |= SHTTPD_END_OF_OUTPUT;
}

static void show_404(struct shttpd_arg *arg)
{
	shttpd_printf(arg, "%s", "HTTP/1.1 200 OK\r\n");
	shttpd_printf(arg, "%s", "Content-Type: text/plain\r\n\r\n");
	shttpd_printf(arg, "%s", "Oops. File not found! ");
	shttpd_printf(arg, "%s", "This is a custom error handler.");
	arg->flags |= SHTTPD_END_OF_OUTPUT;
}

static void show_secret(struct shttpd_arg *arg)
{
	shttpd_printf(arg, "%s", "HTTP/1.1 200 OK\r\n");
	shttpd_printf(arg, "%s", "Content-Type: text/html\r\n\r\n");
	shttpd_printf(arg, "%s", "<html><body>");
	shttpd_printf(arg, "%s", "<p>This is a protected page</body></html>");
	arg->flags |= SHTTPD_END_OF_OUTPUT;
}

static void show_query(struct shttpd_arg *arg)
{
	int		*p = (int*)arg->user_data;	/* integer passed to us */
	char		value[20];
	const char	*host, *request_method, *query_string, *request_uri;
	
	kVSHttpReq req = kVSHttpReqMax;
	void* req_data = NULL;
	void* resp_data = NULL;
	int result = -1;

	request_method = shttpd_get_env(arg, "REQUEST_METHOD");
	request_uri = shttpd_get_env(arg, "REQUEST_URI");
	query_string = shttpd_get_env(arg, "QUERY_STRING");
	
	if (!strcmp(request_method, "POST")) {
		/* If not all data is POSTed, wait for the rest */
		if (arg->flags & SHTTPD_MORE_POST_DATA)
		{
			return;
	    	}

		if (NULL == g_query_callback)
		{
			vshttp_printf("show_query: g_query_callback is null\n");
			resp_500(arg);
			return;			
		}
		    
		result = parse_post_data(arg->in.buf, arg->in.len, &req, &req_data);
		if (result >= 0)
		{
			if (req == kAttackedHostQueryReq)
			{
				resp_data = malloc(sizeof(VSHttpAttackedHostPageResp));
			}
			else
			{
			
			}
		}
		
		if (result < 0)
		{
			vshttp_printf("show_query: parse err\n");
			resp_500(arg);
			return;
		}
		else if (result == 1)
		{
			resp_null_package(arg);
			if (g_query_callback(req, req_data, resp_data) < 0)
			{
				vshttp_printf("show_query: g_query_callback failed\n");
				return;	    
			}
		}
		else
		{
			if (g_query_callback(req, req_data, resp_data) < 0)
			{
				vshttp_printf("show_query: g_query_callback failed\n");
				resp_500(arg);
				return;	    
			}
			resp_one_package(arg, req, resp_data);
		}
		
		if (NULL != resp_data)
		{
			free(resp_data);
			resp_data = NULL;
		}
	} 
}

static void resp_null_package(struct shttpd_arg *arg)
{
	shttpd_printf(arg, "%s",
		"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"
		"null");
	arg->flags |= SHTTPD_END_OF_OUTPUT;
}

static void resp_one_package(struct shttpd_arg *arg, 
	kVSHttpReq req, void* resp_data)
{
	vshttp_printf("resp_one_package\n");
	char* resp_xml = NULL;
	switch (req)
	{
	case kAttackedHostQueryReq:
		resp_xml = build_attacked_host_resp_data((VSHttpAttackedHostPageResp*)resp_data);
		break;
	default:
		break;
	}

	if (NULL != resp_xml)
	{
		shttpd_printf(arg, "%s%s",
		"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n",
		resp_xml);
		arg->flags |= SHTTPD_END_OF_OUTPUT;
		
		free(resp_xml);
		resp_xml = NULL;
	}
	else
	{
		resp_500(arg);
	}

}

static void resp_500(struct shttpd_arg *arg)
{
	shttpd_printf(arg, "HTTP/1.1 500 Server err\r\n");
	arg->flags |= SHTTPD_END_OF_OUTPUT;
}

static void resp_200(struct shttpd_arg *arg)
{
	shttpd_printf(arg, "%s",
		"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");
	arg->flags |= SHTTPD_END_OF_OUTPUT;
}

static int parse_post_data(char* post_data, int post_data_len, kVSHttpReq* req, void** req_data)
{
	int result = -1;

	//xmlDocPtr doc = xmlParseMemory(post_data, post_data_len);
	xmlDocPtr doc = xmlReadMemory(post_data, post_data_len, NULL, NULL, 0);
	if (NULL == doc)
	{
		vshttp_printf("parse_post_data doc is nul\nl");
		return -1;
	}
	
	xmlNodePtr root = xmlDocGetRootElement(doc);
	if (NULL == root)
	{
		vshttp_printf("parse_post_data root is null\n");
		xmlFreeDoc(doc);
		return -1;
	}
	
	if (!xmlStrcmp(root->name, BAD_CAST"attacked_host_query"))
	{
		xmlNodePtr child_node = root->xmlChildrenNode;
		VSHttpAttackedHostReq* attacked_host_data = (VSHttpAttackedHostReq*)malloc(sizeof(VSHttpAttackedHostReq));
		*req = kAttackedHostQueryReq;
		while (child_node != NULL)
		{
			if (!xmlStrcmp(child_node->name, BAD_CAST"cur_page"))
			{
				xmlChar *szKey = xmlNodeGetContent(child_node);
				vshttp_printf("cur_page: %s\n", szKey);
				attacked_host_data->cur_page = atoi((char*)szKey);
				xmlFree(szKey);
			}
			else if (!xmlStrcmp(child_node->name, BAD_CAST"ip"))
			{
				xmlChar *szKey = xmlNodeGetContent(child_node);
				vshttp_printf("ip: %s\n", szKey);
				attacked_host_data->ip = atoi((char*)szKey);
				xmlFree(szKey);
			}
			else if (!xmlStrcmp(child_node->name, BAD_CAST"port"))
			{
				xmlChar *szKey = xmlNodeGetContent(child_node);
				vshttp_printf("port: %s\n", szKey);
				attacked_host_data->port = atoi((char*)szKey);
				xmlFree(szKey);
			}
			else if (!xmlStrcmp(child_node->name, BAD_CAST"start_time"))
			{
				xmlChar *szKey = xmlNodeGetContent(child_node);
				vshttp_printf("start_time: %s\n", szKey);
				attacked_host_data->start_time = atoi((char*)szKey);
				xmlFree(szKey);
			}
			else if (!xmlStrcmp(child_node->name, BAD_CAST"end_time"))
			{
				xmlChar *szKey = xmlNodeGetContent(child_node);
				vshttp_printf("end_time: %s\n", szKey);
				attacked_host_data->end_time = atoi((char*)szKey);
				xmlFree(szKey);
			}
			else if (!xmlStrcmp(child_node->name, BAD_CAST"ext_ip"))
			{
				xmlChar *szKey = xmlNodeGetContent(child_node);
				vshttp_printf("ext_ip: %s\n", szKey);
				attacked_host_data->ext_ip = atoi((char*)szKey);
				xmlFree(szKey);
			}
			else if (!xmlStrcmp(child_node->name, BAD_CAST"atk_type"))
			{
				xmlChar *szKey = xmlNodeGetContent(child_node);
				vshttp_printf("atk_type: %s\n", szKey);
				strcpy(attacked_host_data->atk_type, (char*)szKey);
				xmlFree(szKey);
			}
			else
			{

			}
			
       		child_node = child_node->next;
       	}

		result = 0;

		if (attacked_host_data->cur_page == -1)
		{
			vshttp_printf("cur_page:%d\n",  attacked_host_data->cur_page);
			result = 1;
		}
		
		*req_data = attacked_host_data;
	}
	else
	{
       	result = -1;
	}

	vshttp_printf("parse end %d\n", result);
	xmlFreeDoc(doc);
	doc = NULL;

   	return result;
}


static char* build_attacked_host_resp_data(VSHttpAttackedHostPageResp* resp_data)
{
	char* p = NULL;
	xmlChar *xml_buff = NULL;
	int buffer_size = 0;
	char tmp[32] = {0};
	xmlNodePtr secNode = NULL;
	int i = 0;

	xmlDocPtr doc = xmlNewDoc(BAD_CAST"1.0");
	xmlNodePtr root_node = xmlNewNode(NULL,BAD_CAST"attacked_host_page_resp");
	xmlDocSetRootElement(doc, root_node);
	sprintf(tmp, "%d", resp_data->cur_page);
	xmlNewTextChild(root_node, NULL, BAD_CAST"cur_page", BAD_CAST tmp);

	memset(tmp, 0, sizeof(tmp));
	sprintf(tmp, "%d", resp_data->total_page);
	xmlNewTextChild(root_node, NULL, BAD_CAST"total_page", BAD_CAST tmp);

	memset(tmp, 0, sizeof(tmp));
	sprintf(tmp, "%u", resp_data->cur_num);	
	xmlNewTextChild(root_node, NULL, BAD_CAST"cur_num", BAD_CAST tmp);
	
	for (i = 0; i < resp_data->cur_num; ++i)
	{
		secNode = xmlNewNode(NULL, BAD_CAST "attacked_host_resp");
		xmlAddChild(root_node, secNode);

		memset(tmp, 0, sizeof(tmp));
		sprintf(tmp, "%u", resp_data->resp[i].starttime);			
		xmlNewTextChild(secNode, NULL, BAD_CAST"starttime", BAD_CAST tmp);
		
		memset(tmp, 0, sizeof(tmp));
		sprintf(tmp, "%u", resp_data->resp[i].endtime);			
		xmlNewTextChild(secNode, NULL, BAD_CAST"endtime", BAD_CAST tmp);
		
		memset(tmp, 0, sizeof(tmp));
		sprintf(tmp, "%u", resp_data->resp[i].ip_src);			
		xmlNewTextChild(secNode, NULL, BAD_CAST"ip_src", BAD_CAST tmp);
		
		memset(tmp, 0, sizeof(tmp));
		sprintf(tmp, "%u", resp_data->resp[i].ip_dest);			
		xmlNewTextChild(secNode, NULL, BAD_CAST"ip_dest", BAD_CAST tmp);
		
		memset(tmp, 0, sizeof(tmp));
		sprintf(tmp, "%u", resp_data->resp[i].ip_ext);			
		xmlNewTextChild(secNode, NULL, BAD_CAST"ip_ext", BAD_CAST tmp);
		
		memset(tmp, 0, sizeof(tmp));
		sprintf(tmp, "%u", resp_data->resp[i].ip_src);			
		xmlNewTextChild(secNode, NULL, BAD_CAST"port_src", BAD_CAST tmp);
		
		memset(tmp, 0, sizeof(tmp));
		sprintf(tmp, "%u", resp_data->resp[i].ip_dest);			
		xmlNewTextChild(secNode, NULL, BAD_CAST"port_dest", BAD_CAST tmp);
		
		memset(tmp, 0, sizeof(tmp));
		sprintf(tmp, "%u", resp_data->resp[i].times);			
		xmlNewTextChild(secNode, NULL, BAD_CAST"times", BAD_CAST tmp);
		
		memset(tmp, 0, sizeof(tmp));
		sprintf(tmp, "%u", resp_data->resp[i].protocal);			
		xmlNewTextChild(secNode, NULL, BAD_CAST"protocal", BAD_CAST tmp);
			
		xmlNewTextChild(secNode, NULL, BAD_CAST"atk_type", BAD_CAST resp_data->resp[i].atk_type);
	}	

	xmlDocDumpFormatMemory(doc, &xml_buff, &buffer_size, 1);
	//xmlDocDumpMemory(doc, &xml_buff, &buffer_size);

	p = (char*)malloc(buffer_size + 1);
	memset(p, 0, buffer_size + 1);
	memcpy(p, (char*)xml_buff, buffer_size);
	xmlFree(xml_buff);
	xmlFreeDoc(doc);
	doc = NULL;
	xmlCleanupParser();
	xmlMemoryDump();
	vshttp_printf("BuildRespData:%s\n", p);
	
	return p;
}
