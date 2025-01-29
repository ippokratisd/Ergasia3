CC = gcc
CFLAGS = -Wall -g
TARGET = e-shop

all: $(TARGET)

$(TARGET): e-shop.o
	$(CC) $(CFLAGS) -o $(TARGET) e-shop.o

e-shop.o: e-shop.c e-shop.h
	$(CC) $(CFLAGS) -c e-shop.c

clean:
	rm -f *.o $(TARGET)
