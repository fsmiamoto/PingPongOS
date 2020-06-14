#!/bin/sh
#
# Runs the tests on the given directory
# github.com/fsmiamoto
#
# Example:
# $ ./run.sh $MY_TEST_DIR
#
# Each test is composed of a executable binary and a '.expect.txt' file
# The binaries are executed and the outputs are compared to the correspondent '.expect.txt'
#

TEST_DIR="$1"

[ "$TEST_DIR" = "" ] && echo "No directory given" && exit 1

# Color escape sequences
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
NC='\033[0m'

echo -e "${YELLOW}TESTS${NC}"

for t in $(ls $TEST_DIR/*.bin); do \
    bin=${t%%.bin}
    got="$bin.output.txt"
    expect="$bin.expect.txt"

    $bin > "$got"

    diff --color=auto "$got" "$expect" > /dev/null

    if [ "$?" = "0" ]; then
        echo -e "${GREEN}PASSED${NC}: ${bin}"
    else
        echo -e "${RED}FAILED${NC}: ${bin}"
        diff --color=auto "$got" "$expect"
    fi
done

printf %"$COLUMNS"s |tr " " "-"
