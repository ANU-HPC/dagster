#!/bin/bash
set -e
export GLOG_v=0 # verbose glog logging at level 5 (0 is least verbose)
export GLOG_logtostderr=true # otherwise all logging goes to /tmp/appname.hostname.username.log.INFO.date.pid
echo "COMPILING ALL TESTS"
mpic++ -std=c++17 -o tests -ggdb tests.c -lgtest -lpthread -lglog -Wno-write-strings -lstdc++fs -lcudd -DNO_PROVENANCE -I.
g++ -std=c++17 -o after_test -g after_test.c -lgtest -lpthread -lstdc++fs -lcudd
echo ""
echo "RUNNING ALL TESTS"
valgrind ./tests
echo ""
./after_test
rm tests
rm after_test
rm -r DAG_CNF_DIRECTORY
rm *.tcnf
