CC= $(CROSS_COMPILE)gcc
CFLAGS= -g -Wall
LDFLAGS= -pthread 

TARGET= aesdsocket
SRCS:= aesdsocket.c

default: $(TARGET)
all: $(TARGET)

$(TARGET): $(SRCS) 
	$(CC) $(CFLAGS) $^ -o $@ $(INCLUDES) $(LDFLAGS) 

clean: 	
	rm -f *.o $(TARGET) *.elf *.map
