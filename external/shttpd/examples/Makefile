all:
	echo "make (linux|unix|msvc)"

clean:
	rm -f a.out*

../src/libshttpd.a-linux:
	cd ../src && $(MAKE) linux

../src/libshttpd.a:
	cd ../src && $(MAKE) unix

linux: ../src/libshttpd.a-linux
	$(CC) $(CFLAGS) example.c -I ../src  ../src/libshttpd.a -o example -ldl -lpthread

unix: ../src/libshttpd.a
	$(CC) $(CFLAGS) example.c -I ../src  ../src/libshttpd.a -o example

WIN32_LIBS= user32.lib shell32.lib ws2_32.lib advapi32.lib ..\src\shttpd.lib
CL_FLAGS= /MD /DNDEBUG /nologo /Zi /I ..\src

msvc:
	cl $(CL_FLAGS) example.c $(WIN32_LIBS)
	rc dialog.rc
	cl $(CL_FLAGS) example_win32_dialog_app.c dialog.res $(WIN32_LIBS)
