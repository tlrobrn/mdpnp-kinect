OSTYPE := $(shell uname -s)

BIN_DIR = ../

INC_DIRS = /home/tobrien/Developer/kinect/OpenNI/Include /usr/include/ni ../includes

SRC_FILES = \
	src/monitor.cpp \

EXE_NAME = MDPnP-KinectMonitor

ifneq "$(GLES)" "1"
ifeq ("$(OSTYPE)","Darwin")
	LDFLAGS += -framework OpenGL -framework GLUT
else
	USED_LIBS += glut
endif
else
	DEFINES += USE_GLES
	USED_LIBS += GLES_CM IMGegl srv_um
	SRC_FILES += opengles.cpp
endif

USED_LIBS += OpenNI

LIB_DIRS += /home/tobrien/Developer/kinect/OpenNI/Lib
include /home/tobrien/Developer/kinect/OpenNI/Include/CommonCppMakefile

