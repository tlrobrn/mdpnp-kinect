PROGRAM = MDPnP-KinectMonitor

INCLUDEDIRS =	\
	-I./includes		\
	-I/usr/include/ni

LIBDIRS = -L/usr/lib

LIBS = -lOpenNI

CPPSOURCES =	\
	main.cpp		\
	src/monitor.cpp

CPPOBJECTS =	\
	bin/main.o	\
	bin/monitor.o

CPPFLAGS = -DESRI_UNIX $(INCLUDEDIRS)

CPP = g++

LDFLAGS = $(LIBDIRS) $(LIBS)

all:	$(PROGRAM)

$(PROGRAM):	$(CPPOBJECTS)
	$(CPP) -o $@ $(CPPOBJECTS) $(LDFLAGS)


bin/main.o:	main.cpp includes/monitor.h
	$(CPP) $(CPPFLAGS) -c -o bin/main.o main.cpp

bin/monitor.o:	src/monitor.cpp includes/monitor.h
	$(CPP) $(CPPFLAGS) -c -o bin/monitor.o src/monitor.cpp

clean:
	$(RM) $(CPPOBJECTS) $(PROGRAM)
