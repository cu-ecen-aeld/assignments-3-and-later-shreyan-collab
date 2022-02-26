CC= $(CROSS_COMPILE)gcc
CFLAGS= -g -Wall -Werror
LDFLAGS= -pthread -lrt
TARGET= aesdsocket

default: $(TARGET)
all: $(TARGET)
	$(CC) $(CFLAGS) $(LDFLAGS) -c -o $(TARGET).o $(TARGET).c
	$(CC) $(CFLAGS) $(LDFLAGS) -I/ $(TARGET).o -o $(TARGET)

clean: 	
	rm -f *.o $(TARGET) *.elf *.map
	
	
