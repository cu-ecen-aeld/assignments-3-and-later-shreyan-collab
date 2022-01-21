CC= $(CROSS_COMPILE)gcc
CFLAGS= -g -Wall 
TARGET= writer

all: $(TARGET)
	$(CC) $(CFLAGS) -c -o $(TARGET).o $(TARGET).c
	$(CC) $(CFLAGS) -I/ $(TARGET).o -o $(TARGET)

clean: 	
	rm -f *.o $(TARGET) *.elf *.map
	
	
