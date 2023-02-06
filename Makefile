TARGET = pspfps
OBJS = main.o display/draw.o display/vram.o system/callbacks.o

INCDIR =
CFLAGS = 

LIBDIR =
LDFLAGS =
LIBS= -lpspgum -lpspgu

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = PSPFPS

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak


