DIRECT := /opt/samygo/oe/trunk/build-samygo/tmp/cross/bin/
CROSS := $(DIRECT)arm-linux-gnueabi-
CC := $(CROSS)gcc
STRIP := $(CROSS)strip
LD	:= $(CROSS)ld

CFLAGS += -Wall -O2
CFLAGS += -I./
CFLAGS += -I./include
CFLAGS += -I/opt/samygo/oe/trunk/build-samygo/tmp/staging/arm-linux-gnueabi/include
LDFLAGS += -L/opt/samygo/oe/trunk/build-samygo/tmp/staging/arm-linux-gnueabi/lib
LIB += -lSDL_ttf -lfreetype

all: libpopup.so

libpopup.so: popup.o
	$(AR) -x /opt/samygo/oe/trunk/build-samygo/tmp/staging/arm-linux-gnueabi/lib/libSDL_ttf.a
	$(AR) -x /opt/samygo/oe/trunk/build-samygo/tmp/staging/arm-linux-gnueabi/lib/libfreetype.a
	$(AR) -cr libpopup.a *.o
	$(LD) $(LDFLAGS) -shared -soname,$@ -o $@ $^ -static $(LIB)
	$(STRIP) $@

clean:
	rm -f *.o *.a *.so *~

svnup: clean
	svn commit -m "Zmiany zmiany zmiany"

