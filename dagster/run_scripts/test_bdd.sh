#!/bin/bash

# need to change loops so that the master loop using bddmaster
# to print all bdds to file, set the first argument on line 170 in loops.cpp
# to print all cudd memory stats, set the second argument on line 170 in loops.pp

# test using costas problems with slightly changed dags - interface is just one level of pyramid
make clean; make -j8 -s
export OMPI_MCA_btl=self,tcp
export GLOG_v=0 # verbose glog logging at level 5 (0 is least verbose)
export GLOG_logtostderr=true # otherwise all logging goes to /tmp/appname.hostname.username.log.INFO.date.pid
mpirun -n 4 ./dagster -m 0 costas_10.dag costas_10.cnf
exit 0

# test using random 3SAT cnfs:
cd ../Benchmarks/gensat
./build.sh
vars=100
# the ratio for hardest problems is estimated to be around 4.24
clauses=424
num=100
# randomly generate num 3-SAT cnfs with v variables and
./ggen "${vars}" "${clauses}" 3 0 "${num}" 2> /dev/null
# make gnovelty
make -j8 -C ../../Originals/gnovelty+-T/
# extract the SAT random cnfs
touch SATlist.txt
for f in v$v* ; do
  if [ "$(timeout .1 ../../Originals/gnovelty+-T/gnovelty+ $f  2> /dev/null | grep SATISFIABLE)" ]; then
    echo $f >> SATlist.txt
  fi
done
echo "" >> SATlist.txt

# how many nodes?
nodes=5

# make a big cnf file? - use nodes number of random cnf files
touch full_cnf.cnf
echo "p cnf ${vars} $((424*${nodes}))" >> full_cnf.cnf

iter=0
while IFS= read -r line ; do
  if (( iter >= "${nodes}" )) ;  then
    break
  fi
  tail -n +3 "${line}" >> full_cnf.cnf
  (( iter++ ))
done < SATlist.txt
# create appropriate dag file - randomly chose 5% (?) of the variables to pass up the dag
echo "DAG-FILE
NODES:${nodes}
GRAPH:" > test.dag
for (( i=0; i<$((${nodes}-1)); i++ )) ; do
  # randomly choose 5% of the variables to pass on arcs
  # stop repeated literals on an arc?
  var_array=()
  for j in {0..4} ; do
    var=$((RANDOM % 150))
    var_array+=( $var )
  done
  # sort the array:
  sorted=$(printf "%s\n" ${var_array[@]} | sort -n | tr '\n' ' ')
  echo "${i}->$(($i +1)):${sorted[@]}" | tr ' ' ','  >> test.dag
done
# remove trailing commas
sed -i 's/,*$//g'  test.dag
echo "CLAUSES:" >> test.dag
# add all the clauses 
for (( i=0; i<${nodes}; i++ )) ; do
  j=$(( 424 * $i ))
  k=$(( $k + 423 ))
  echo "$i:$j-$k" >> test.dag
done

cat test.dag

# run dagster on the created cnf and dag
mpirun -n 4 -m 0 ../../dagster/dagster test.dag full_cnf.cnf



# clean up after each run
rm -r TEMP_CNF_DIRECTORY
rm SATlist.txt
rm test.dag
rm full_cnf.cnf
rm v*
rm ggen
make clean -C ../../Originals/gnovelty+-T/
