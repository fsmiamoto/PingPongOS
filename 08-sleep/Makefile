CC = clang
CFLAGS = -Wall
LD = ld
LDFLAGS = --relocatable
OBJ = ppos.o

TEST_DIR= ./tests
TEST_FLAGS= -g3 # all debug info

all: ppos_core.o queue.o
	$(LD) $(LDFLAGS) $^ -o $(OBJ)

ppos_core.o: ppos_core.c ppos.h ppos_data.h ppos_internal.h
	$(CC) $(CFLAGS) -c $< -o $@

queue.o: queue.c queue.h
	$(CC) $(CFLAGS) -c $< -o $@

$(TEST_DIR)/%.c: all
	$(CC) $(CFLAGS) -DDEBUG $(TEST_FLAGS) $@ $(OBJ) -o $(@:.c=.bin)

build_test: $(TEST_DIR)/*.c

test: $(TEST_DIR)/*.c
	$(TEST_DIR)/bin/run-tests.sh $(TEST_DIR)

clean:
	rm -rf *.o
	rm -rf $(TEST_DIR)/*.bin $(TEST_DIR)/*.output.txt


