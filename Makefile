CC      = gcc
CFLAGS  = -Wall -Wextra -g -O0
LDFLAGS = -pthread

SRC = $(wildcard src/*.c)
OBJ = $(SRC:.c=.o)

simulador: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f simulador src/*.o *.log

.PHONY: clean
