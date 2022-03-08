#!/bin/bash
set -e
echo "RUNNING ALL TESTS"
(cd "../minimal/" && bash run.sh -H)
for d in ./*/ ;
  do (echo "$d" && cd "$d" && bash run.sh -H);
done
./cleanup.sh
echo "DONE"
