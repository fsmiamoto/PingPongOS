CC = gcc
CFLAGS = -Wall
TESTS = tasks1 tasks2 tasks3
OBJ = ppos_core.o

all: ppos_core.c ppos.h ppos_data.h
	$(CC) $(CFLAGS) -c ppos_core.c -o $(OBJ)

tasks%: all pingpong-tasks%.c
	$(CC) $(CFLAGS) pingpong-$@.c $(OBJ) -o $@

test: $(TESTS)
	@echo "--------------------------------------------------------------------------------"
	@echo "TESTS: $(TESTS)"
	@echo "--------------------------------------------------------------------------------"
	@for t in $(TESTS); do \
		./"$$t" > "$$t.output.txt"; \
		./assert_equal.sh "$$t.output.txt" "$$t.expect.txt"; \
		rm "$$t.output.txt"; \
	done
	@echo "--------------------------------------------------------------------------------"

clean:
	rm -rf *.o $(TESTS)


