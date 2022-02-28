CC= $(CROSS_COMPILE)gcc
CFLAGS= -g -Wall
LDFLAGS= -pthread 

TARGET= aesdsocket
SRCS:= aesdsocket.c

default: $(TARGET)
all: $(TARGET)

$(TARGET): $(SRCS) 
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@ $(INCLUDES) 

clean: 	
	rm -f *.o $(TARGET) *.elf *.map
