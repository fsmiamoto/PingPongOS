CC = gcc
CFLAGS = -Wall
TEST_DIR= ./tests
OBJ = ppos_core.o

all: ppos_core.c ppos.h ppos_data.h
	$(CC) $(CFLAGS) -c ppos_core.c -o $(OBJ)

$(TEST_DIR)/%.c: all
	$(CC) $(CFLAGS) $@ $(OBJ) -o $(@:.c=.bin)

test: $(TEST_DIR)/*.c
	@$(TEST_DIR)/bin/run-tests.sh $(TEST_DIR)

clean:
	rm -rf *.o
	rm -rf $(TEST_DIR)/*.bin $(TEST_DIR)/*.output.txt


