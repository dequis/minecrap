CFLAGS=$(shell pkg-config --cflags glib-2.0 gio-2.0) -g
LDFLAGS=$(shell pkg-config --libs glib-2.0 gio-2.0)

minecrap: minecrap.o packets.o types.o buf.o util.o

clean:
	rm -f minecrap *.o
