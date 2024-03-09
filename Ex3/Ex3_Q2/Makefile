CC = gcc
CFLAGS = -Wall -pthread
LIBS = -lm

TARGET = Q2

all: $(TARGET)

$(TARGET): Q2.c
	$(CC) $(CFLAGS) -o $(TARGET) Q2.c $(LIBS)

clean:
	rm -f $(TARGET)

