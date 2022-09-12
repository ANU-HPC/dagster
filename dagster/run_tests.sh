#!/bin/bash
set -e
make clean
make
(cd "./tests/minimal/" && bash run.sh -H)
echo "DONE"
