# xxx APP Makefile 
#
#
#
#
#

VERSION = 1
PATCHLEVEL = 0
SUBLEVEL = 0
EXTRAVERSION =
NAME = Series video top1

APP_TOPDIR = $(shell pwd)
FFMPEG_VER = FFmpegV4.0
FFMPEG_DIR = $(APP_TOPDIR)/$(FFMPEG_VER)

CROSS_COMPILE =
CC = $(CROSS_COMPILE)gcc
CXX = $(CROSS_COMPILE)g++
AR = $(CROSS_COMPILE)ar
LD = $(CROSS_COMPILE)ld

CFLAGS = -g -O2 -W -Wall
CPPFLAGS =
LIB_INC = -I$(FFMPEG_DIR)/include
C_INC = -I.
INCLUDES = $(C_INC) $(LIB_INC)
LIBS = -L$(FFMPEG_DIR)/lib \
	   -lavformat -lavcodec -lavutil -lswresample -lm -lpthread -lz 

OUTPUT = $(APP_TOPDIR)/bin
OBJS = ivDisDemo.o test.o
SUB_DIRS = 
TARGET = videoDisD

export OUTPUT

.PHONY : clean

all : CHECKDIR $(SUB_DIR) $(TARGET)

CHECKDIR :
	mkdir -p $(SUB_DIR) $(OUTPUT)

$(SUB_DIR) : ECHO
	make -C $@

ECHO :
	@echo $(SUB_DIR)
	@echo " begin compile"

$(TARGET) : $(OBJS)
	@echo "LD $@ ......"
	@$(CC) $(OUTPUT)/*.o $(LIBS) -o $(OUTPUT)/$(TARGET)

%.o : %.c
	@echo "$(CC) $*.o"
	echo " path = $(INCLUDES)"
	@$(CC) -c $(CFLAGS) $(INCLUDES) $< -o $(OUTPUT)/$@

clean :
mrproper :
	rm -rf $(OUTPUT)/*.o $(OUTPUT)/$(TARGET)

distclean :
clobber:
	rm -rf $(OUTPUT)
