CFLAGS=-std=c99 -pedantic -Wall `pkg-config --cflags xaw7`
LFLAGS=`pkg-config --libs xaw7`

.PHONY: clean debug

xii: xii.c
	gcc $(CFLAGS) $(LFLAGS) -o $@ xii.c

clean:
	rm -f xii xii.core

debug:
	gdb xii xii.core
