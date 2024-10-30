TARGET = pspfps
OBJS = src/main.o src/texture.o src/navmesh.o src/display/draw.o src/display/vram.o src/system/callbacks.o src/bwm.o src/audio.o \
		src/camera.o src/stage2.o src/filesystem.o src/controls.o
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