#!/bin/bash
set -e
make clean
make
(cd "./tests/comprehensive/" && bash run.sh -H)
echo "DONE"
