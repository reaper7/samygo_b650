DIRECT := /opt/samygo/oe/trunk/build-samygo/tmp/cross/bin/
CROSS := $(DIRECT)arm-linux-gnueabi-
CC := $(CROSS)gcc
STRIP := $(CROSS)strip
LD	:= $(CROSS)ld
AR	:= $(CROSS)ar

CFLAGS += -Wall -O2
CFLAGS += -I./
CFLAGS += -I./include
CFLAGS += -I/opt/samygo/oe/trunk/build-samygo/tmp/staging/arm-linux-gnueabi/include
LDFLAGS += -L../popup
LDFLAGS += -L/opt/samygo/oe/trunk/build-samygo/tmp/staging/arm-linux-gnueabi/lib
LIB += -lpopup -lSDL_ttf -lfreetype

PLUGNAME=mybuttons
APP_DIR=/dtv/usb/sda/$(PLUGNAME)
FILES=*.so addons/*.png addons/clmeta.dat addons/mybuttons.conf.default addons/mybuttons.conf addons/rc_codes.html
SRC=include *.c Makefile

all: loader.so $(PLUGNAME).so

loader.so: loader.o
	$(LD) $(LDFLAGS) -shared -soname,$@ -o $@ $^
	$(STRIP) $@

$(PLUGNAME).so: $(PLUGNAME).o
	$(LD) $(LDFLAGS) -shared -soname,$@ -o $@ $^ -static $(LIB)
	$(STRIP) $@

clean:
	rm -rf *.so *.o *.a *~ ./src ./mybuttons

install:
	ncftpput -u root -p "" 192.168.0.9 $(APP_DIR) $(FILES)

targz:
	mkdir -p ./$(PLUGNAME)
	cp $(FILES) ./$(PLUGNAME)
	mkdir -p ./src
	cp -R $(SRC) ./src
	tar cvfz samygo-${PLUGNAME}-plugin.tar.gz  $(PLUGNAME)/* src/*

svnup: clean
	svn commit -m 'Some changes...'

