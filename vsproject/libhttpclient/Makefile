SRCS=		vshttpclient.cpp HttpReqBase.cpp HttpRequest.cpp AttackedHostHttpReq.cpp
HDRS=		vshttpclient.h
OBJS=		$(SRCS:%.cpp=%.o)
PROG=		vshttpclient
PROG_TEST=  vshttpclient_test
CC= g++

all:
	@echo "make linux"

.cpp.o:
	$(CC) -c $(CFLAGS) $< -o $@ \
		-I../../external/curl/include \
		-I../../external/libxml2/include

lib$(PROG).a:$(OBJS) 
	$(AR) -r lib$(PROG).a $(OBJS)
	ranlib lib$(PROG).a

linux:lib$(PROG).a
	$(CC) $(CFLAGS) \
	vshttpclient_test.c \
	-o $(PROG_TEST)	$(LIBS) \
	-L. -l$(PROG) \
	-L../../external/curl/lib/.libs -lcurl \
	-L../../external/libxml2/.libs -lxml2 \
	-ldl -lpthread

libcurl:
	cd ../../external/curl/ && ./configure && make

libxml2:
	cd ../../external/libxml2/ && ./autogen.sh --without-zlib && make

clean:
	rm -rf *.o *.core $(PROG) $(PROG_TEST) lib$(PROG).a *.lib *.obj
