#all: 
#	g++ control.cpp dumpData.cpp itg3200.cpp PruProxy.cpp -o control

CC=g++
#CFLAGS=
LDFLAGS=-L $(LIBDIR_APP_LOADER)
LIBDIR_APP_LOADER?=../../../app_loader/lib
INCDIR_APP_LOADER?=../../../app_loader/include
CFLAGS=-c -w -ggdb -Wall -I $(INCDIR_APP_LOADER) 
SOURCES=control.cpp dumpData.cpp sensor.cpp itg4200.cpp PruProxy.cpp misc.cpp config.cpp adxl345.cpp gpio.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=control

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS)  -lrt -lpthread -lprussdrv -o $@ 

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@
	
