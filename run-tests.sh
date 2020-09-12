#!/bin/sh
#
# Runs the tests on the given directory
# github.com/fsmiamoto
#
# Each test is composed of a executable file '.bin' and a '.expect.txt' file
# The binaries are executed and the outputs are compared to the correspondent '.expect.txt'
#
# Example:
# Given following file tree:
# ./tests
# ├── dispatcher.bin
# ├── dispatcher.expect.txt
# ├── tasks.bin
# └── tasks.expect.txt
#
# You can run the following:
# $ ./run.sh ./tests

TEST_DIR="$1"

[ "$TEST_DIR" = "" ] && echo "No directory given" && exit 1

# Color escape sequences
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
NC='\033[0m'

DIFF_TOOL='diff --color=auto'

if command -v delta; then
    DIFF_TOOL='delta --dark --side-by-side'
fi

echo -e "${YELLOW}TESTS${NC}"

for bin in $(ls $TEST_DIR/*.bin); do \
    name=${bin%%.bin}
    got="$name.output.txt"
    expect="$name.expect.txt"

    echo "Running $name..."
    $bin | tee "$got"

    # Here, always use diff just for the check
    diff -s "$got" "$expect" > /dev/null

    if [ ! "$?" = "0" ]; then
        echo -e "${RED}FAILED${NC}: ${name}"
        $DIFF_TOOL "$got" "$expect"
        continue
    fi

    echo -e "${GREEN}PASSED${NC}: ${name}"
done

printf %"$COLUMNS"s |tr " " "-"
