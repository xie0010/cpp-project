CC = gcc
CFLAGS = -std=c99 -Wall
LDFLAGS = -lsqlite3
TARGET = main
SRC = main.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)


clean:
	rm -f $(TARGET)
