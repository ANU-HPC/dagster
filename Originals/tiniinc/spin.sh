#!/bin/bash

# This is a script C. Gretton wrote that can be used to sanity check the Dagster results.
# It is a very primitive model enumeration procedure based on TiniSAT. 

make

mydir=`pwd`
cd ../Originals/tinisat0.22/
make
cp tinisat $mydir/
cd $mydir


if [ "$#" -ne 1 ]
then
  echo "Usage: $0 N|FILENAME" >&2
  exit 1
fi
N=$1


if [ -f thing.cnf ] ; then
    echo "NOTE: deleting the file thing.cnf" >&2
    rm thing.cnf
fi


re='^[0-9]+$'
if ! [[ $N =~ $re ]] ; then
    (>&2 echo "Interpreting input $N as a filename.")
fi

if [[ $N =~ $re ]] ; then
    if [ ! -f  ../../Benchmarks/costas/costas_${N}.dag ] ||  [ ! -f  ../../Benchmarks/costas/costas_${N}.cnf ] ; then
	(>&2 echo "Requested costas problem file does not seem to exist, we shall generate it now...")
	MYDIR=`pwd`
	cd ../../Benchmarks/costas/
	if [ ! -f generate_costas_N ] ; then
	    make
	fi
	
	./generate_costas_N -N ${N} > costas_${N}.cnf 2> costas_${N}.mapping
	
	cd ${MYDIR}
	
	
	if [ ! -f  ../../Benchmarks/costas/costas_${N}.dag ]  &&  [ ! -f  ../../Benchmarks/costas/costas_${N}.cnf ] ; then
	    (>&2 echo "The input problem file for the requested costas problem could not be generated. Exiting.  ")
	    exit 1
	fi
	
	(>&2 echo "The input file should be generated now, continuing. ")
    fi
    

    if [ ! -f ../../Benchmarks/costas/costas_${N}.cnf ] ; then
	echo "UNRECOVERABLE ERROR: no input file ../../Benchmarks/costas/costas_${N}.cnf" >&2
	exit 1
    fi
    
    cp ../../Benchmarks/costas/costas_${N}.cnf thing.cnf
    
    MODELS_ANNOTATION=`echo "$N"`
else
    cp $N thing.cnf
    MODELS_ANNOTATION=`echo "$N" | sed s/"\.\.\/"/""/g | sed s/"\/"/"\."/g`
fi

if [ -f costas.$MODELS_ANNOTATION.models ] ; then
    rm  costas.$MODELS_ANNOTATION.models
fi
    
touch costas.$MODELS_ANNOTATION.models

COUNT=0

while true ; do
    #DEBUG#wc -l thing.cnf
    #DEBUG#head -1 thing.cnf
    ./tinisat thing.cnf > OUTPUT
    time ./tinisat thing.cnf > /dev/null 2> tinisat.time
    time mpirun -n 4 ./mpi_launcher  -d 100 -s 30 -r cdclfirst -a ghosts thing.cnf > /dev/null 2> hybrid.time

    cat tinisat.time
    cat hybrid.time
#    exit -1 
    
    if cat OUTPUT | grep -q "s SATISFIABLE" ; then  # I should probably not be calling a SAT solver twice for each solution
	OLDNUMCLAUSES=`cat thing.cnf | grep ^p | awk '{print $4}'`
	NEWNUMCLAUSES=$(($OLDNUMCLAUSES + 1))
	cat OUTPUT | grep -v "Time " | grep 0$ | tr ' ' '\n' | sed -E 's/^([^\-])/+\1/g'| sed s/"\-"/""/g | sed s/+/"\-"/g | tr '\n' ' ' | sed s/"\-0"/" 0"/g > new_clause.cnf
	
	echo $COUNT
	echo `cat new_clause.cnf` >> thing.cnf
	echo `cat new_clause.cnf` >> costas.$MODELS_ANNOTATION.models
	cat thing.cnf | sed s/"$OLDNUMCLAUSES"/"$NEWNUMCLAUSES"/g > tmp.cnf
	mv tmp.cnf thing.cnf
	COUNT=$(($COUNT + 1))
    else
	break;
    fi
done

echo "Number of models found for coastas $N is: $COUNT"
echo "These have been stored is the 'nogood' clauses at: costas.$MODELS_ANNOTATION.models"
wc -l costas.$MODELS_ANNOTATION.models
