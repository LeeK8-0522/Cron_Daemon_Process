TARGET = cron
CC = gcc
CFLAGS = -Wall -g 

all: $(TARGET)

$(TARGET): cron.c
	$(CC) $(CFLAGS) -o $(TARGET) cron.c 

clean:
	rm -f $(TARGET)
