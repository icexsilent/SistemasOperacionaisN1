CC      = gcc
CFLAGS  = -Wall -Wextra -g -O0 -pthread
LDFLAGS = -pthread
PYTHON ?= python3

SRC = $(wildcard src/*.c)
OBJ = $(SRC:.c=.o)
EXEEXT =
ifeq ($(OS),Windows_NT)
EXEEXT = .exe
endif
TARGET = simulador$(EXEEXT)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LDFLAGS)

%.o: %.c src/args.h src/log.h
	$(CC) $(CFLAGS) -c $< -o $@

test: $(TARGET)
	$(PYTHON) testar.py

clean:
	rm -f simulador simulador.exe src/*.o *.log

.PHONY: all test clean
