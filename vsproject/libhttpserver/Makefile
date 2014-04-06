SRCS=		vshttpserver.c
HDRS=		vshttpserver.h
OBJS=		$(SRCS:%.c=%.o)
PROG=		vshttpserver
PROG_TEST=  vshttpserver_test
CC=g++

all:
	@echo "make linux"

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@ \
		-I ../../external/shttpd/src  \
		-I ../../external/libxml2/include

lib$(PROG).a:$(OBJS) 
	$(AR) -r lib$(PROG).a $(OBJS)
	ranlib lib$(PROG).a

linux:lib$(PROG).a
	$(CC) $(CFLAGS) \
	vshttpserver_test.c \
	-o $(PROG_TEST)	$(LIBS) \
	-L. -l$(PROG) \
	-L../../external/shttpd/src -lshttpd \
	-L../../external/libxml2/.libs -lxml2 \
	-ldl -lpthread

clean:
	rm -rf *.o *.core $(PROG) $(PROG_TEST) lib$(PROG).a *.lib *.obj
