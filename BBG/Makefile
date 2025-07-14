C = arm-linux-gnueabihf-gcc
CFLAGS = -Wall -Wextra -ggdb
LDFLAGS = -lpthread

OBJS = main.o
TARGET = parksys

all: $(TARGET)

$(TARGET): $(OBJS)
	$(C) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(C) $(CFLAGS) -c $< -o $@

PHONY: clean upload

upload:
	scp $(TARGET) debian@192.168.7.2:/home/debian

clean:
	rm *.o