CFLAGS=-std=c99 -pedantic -Wall `pkg-config --cflags xaw7`
LFLAGS=`pkg-config --libs xaw7`

.PHONY: clean debug

all: xii mii

xii: xii.c
	gcc $(CFLAGS) $(LFLAGS) -o $@ xii.c

mii: mii.c
	gcc $(CFLAGS) -I/usr/local/include/ -L/usr/local/lib $(LFLAGS) -lXm -o $@ mii.c

clean:
	rm -f xii xii.core

debug:
	gdb xii xii.core
