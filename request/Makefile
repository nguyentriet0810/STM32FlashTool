CC = gcc
CFLAGS = -Wall -g
OBJ = main.o com.o RWfile.o minprotocol.o
TARGET = request

all: $(TARGET)

#gcc main.c -o main.exe -lws2_32
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ) -lws2_32

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	cmd /c del /F /Q $(OBJ) $(TARGET) *.exe 2>nul