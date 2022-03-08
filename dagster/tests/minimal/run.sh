#!/bin/bash
set -e
echo "RUNNING ALL TESTS"
for d in ./*/ ;
  do (echo "$d" && cd "$d" && bash run.sh -H);
done
./cleanup.sh
echo "DONE"
