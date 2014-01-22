CROSS_COMPILE?=arm-angstrom-linux-gnueabi-
APP_NAME?=camera
LIBDIR_APP_LOADER?=../../app_loader/lib
INCDIR_APP_LOADER?=../../app_loader/include
BINDIR?=../bin

CFLAGS+= -Wall -I$(INCDIR_APP_LOADER) -D__DEBUG -O2 -mtune=cortex-a8 -march=armv7-a -g
LDFLAGS+=-L$(LIBDIR_APP_LOADER) -lprussdrv -lpthread -lm
OBJDIR=obj
TARGET=$(BINDIR)/$(APP_NAME)

_DEPS = 
DEPS = $(patsubst %,$(INCDIR_APP_LOADER)/%,$(_DEPS))

_OBJ = $(APP_NAME).o
OBJ = $(patsubst %,$(OBJDIR)/%,$(_OBJ))


$(OBJDIR)/%.o: %.c $(DEPS)
	@mkdir -p obj
	$(CROSS_COMPILE)gcc $(CFLAGS) -c -o $@ $< 

$(TARGET): $(OBJ)
	$(CROSS_COMPILE)gcc $(CFLAGS) -o $@ $^ $(LDFLAGS)

.PHONY: clean

clean:
	rm -rf $(OBJDIR)/ *~  $(INCDIR_APP_LOADER)/*~  $(TARGET)
