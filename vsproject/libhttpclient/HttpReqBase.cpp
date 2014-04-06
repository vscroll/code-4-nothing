#include "HttpReqBase.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include<time.h>

HttpReqBase::HttpReqBase()
{
	InitId(REQ_ID_LEN);
}

HttpReqBase::~HttpReqBase()
{

}

void HttpReqBase::InitId(int len)
{
	int i = 0;
	srand(time(NULL) + rand());
	for (i = 0; i < len-1; i++)
	{
		mId[i] = rand()%94+32;
	}
	mId[i] = 0;
	vshttp_printf("InitId:%s\n", mId);
}
