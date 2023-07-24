CC := gcc

CFlags := -Wall -std=c99

SRC_DIR := ./src

SRCS := $(wildcard $(SRC_DIR)/*.c)

OUTPUT := game

all: $(OUTPUT)

$(OUTPUT): $(SRCS)
	$(CC) $(CFlags) $(SRCS) -o $(OUTPUT)

clean:
	rm -f $(OUTPUT)
