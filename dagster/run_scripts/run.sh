#!/bin/bash
export OMPI_MCA_btl=self,tcp
export GLOG_v=3 # verbose glog logging at level 5 (0 is least verbose)
export GLOG_logtostderr=true # otherwise all logging goes to /tmp/appname.hostname.username.log.INFO.date.pid
mpirun -n 2 ../dagster ../unit_testing/good/d1.txt ../unit_testing/good/c1.txt
