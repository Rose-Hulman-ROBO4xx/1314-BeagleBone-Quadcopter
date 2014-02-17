#CROSS_COMPILE?=arm-linux-gnueabihf-#arm-linux-gnueabihf#arm-angstrom-linux-gnueabi-
CROSS_COMPILE?=arm-angstrom-linux-gnueabi-

LIBDIR_APP_LOADER?=../../app_loader/lib -L/usr/xenomai/lib
INCDIR_APP_LOADER?=../../app_loader/include
INCDIR_XENOMAI?= /usr/xenomai/include
BINDIR?=../bin

CFLAGS+= -Wall -I$(INCDIR_APP_LOADER) -I$(INCDIR_XENOMAI) -D__DEBUG -O2 -mtune=cortex-a8 -march=armv7-a -g
LDFLAGS+=-L$(LIBDIR_APP_LOADER) -lprussdrv -lpthread -lm -lnative -lxenomai -lrt -lpthread
OBJDIR=obj
TARGET=$(BINDIR)/control_alg

_DEPS = 
DEPS = $(patsubst %,$(INCDIR_APP_LOADER)/%,$(_DEPS))

_OBJ = control_alg.o
OBJ = $(patsubst %,$(OBJDIR)/%,$(_OBJ))


$(OBJDIR)/%.o: %.c $(DEPS)
	@mkdir -p obj
	$(CROSS_COMPILE)gcc $(CFLAGS) -c -o $@ $< 

$(TARGET): $(OBJ)
	$(CROSS_COMPILE)gcc $(CFLAGS) -o $@ $^ $(LDFLAGS)

.PHONY: clean

clean:
	rm -rf $(OBJDIR)/ *~  $(INCDIR_APP_LOADER)/*~  $(TARGET)
