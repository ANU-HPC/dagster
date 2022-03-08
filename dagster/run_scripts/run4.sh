export OMPI_MCA_btl=self,tcp
export GLOG_v=0 # verbose glog logging at level 5 (0 is least verbose)
export GLOG_logtostderr=true # otherwise all logging goes to /tmp/appname.hostname.username.log.INFO.date.pid
mpirun -n 3 ../dagster -m 1 -k 1 ../../Benchmarks/costas/debugging/costas_5.dag ../../Benchmarks/costas/debugging/costas_5.cnf

