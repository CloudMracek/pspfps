TARGET = pspfps
OBJS = main.o texture.o display/draw.o display/vram.o system/callbacks.o inc/bwm.o audio.o

INCDIR =
CFLAGS = 

LIBDIR =
LDFLAGS =
LIBS= -lpspgum -lpspgu -lpspaudio -lpspmp3

EXTRA_TARGETS = EBOOT.PBP

PSP_EBOOT_TITLE = PSPFPS
PSP_EBOOT_ICON=ICON0.png
PSP_EBOOT_SND0=SND0.at3
PSP_EBOOT_PIC1=PIC1.png

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak