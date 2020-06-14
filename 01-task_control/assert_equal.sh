#!/bin/sh

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m'

diff --color=auto "$1" "$2" > /dev/null
if [ "$?" = "0" ]; then
    echo -e "${GREEN}PASSED${NC}: ${1%%.output.txt}"
else
    echo -e "${RED}FAILED${NC}: ${1%%.output.txt}"
    diff --color=auto "$1" "$2" | head -20
fi
