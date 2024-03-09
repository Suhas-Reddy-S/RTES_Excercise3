CC = gcc
CFLAGS = -Wall -pthread
LIBS = -lm

TARGET = Q5

all: $(TARGET)

$(TARGET): Q5.c
	$(CC) $(CFLAGS) -o $(TARGET) Q5.c $(LIBS)

clean:
	rm -f $(TARGET)

