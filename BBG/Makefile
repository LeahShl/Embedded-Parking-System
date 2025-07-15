C = arm-linux-gnueabihf-gcc
CFLAGS = -Wall -Wextra -ggdb -IInc
LDFLAGS = 

SRCDIR = Src
INCDIR = Inc
OBJDIR = Obj

SOURCES := $(wildcard $(SRCDIR)/*.c)
OBJECTS := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SOURCES))
TARGET := parksys

.PHONY: all clean upload

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(C) $(OBJECTS) -o $@ $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(C) $(CFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

upload:
	scp $(TARGET) debian@192.168.7.2:/home/debian

clean:
	rm -rf $(OBJDIR) $(TARGET)