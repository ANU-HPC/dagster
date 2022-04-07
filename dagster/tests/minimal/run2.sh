#!/bin/bash
set -e
echo "RUNNING ALL TESTS"
for d in ./4sat_180_1_checkpointing/ ;
  do (echo "$d" && cd "$d" && bash run.sh -H);
done
#./cleanup.sh
echo "DONE"
