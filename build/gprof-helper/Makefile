# Make gprof-helper library

gprof-helper.so: gprof-helper.c
	gcc -shared -fPIC gprof-helper.c -o gprof-helper.so -lpthread -ldl

install: gprof-helper.so
	cp gprof-helper.so /usr/local/lib/
	cp profile /usr/local/bin/

clean:; @-rm -f gprof-helper.so *~
