#!/bin/bash

# This is a script C. Gretton wrote that can be used to sanity check the Dagster results.
# It is a very primitive model enumeration procedure based on TiniSAT. 

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

cp $N thing.cnf

COUNT=0

while true ; do
    #DEBUG#wc -l thing.cnf
    #DEBUG#head -1 thing.cnf

    #cat thing.cnf
    echo "Running on thing "
    #python check_cnf.py thing.cnf
    mpirun -np 1 ~/dsyrup2/Manager/bin/manager ./thing.cnf ~/dsyrup2/Config/MampharosSAT.xml > /dev/null 2> OUTPUT
    echo "Finished running dsyrup"
    #exit 0;
    cat OUTPUT | sed s/"\-No\ \-protocol\ \-specified"/""/g > TMP
    mv TMP OUTPUT
    #cat OUTPUT
    #exit -1 
    
    if cat OUTPUT | grep -q ^v ; then  # I should probably not be calling a SAT solver twice for each solution
	echo "OUTPUT has a v"
	OLDNUMCLAUSES=`cat thing.cnf | grep ^p | awk '{print $4}'`
	NEWNUMCLAUSES=$(($OLDNUMCLAUSES + 1))
	cat OUTPUT | grep "v" | tr ' ' '\n' | grep -v "v" | sed -E 's/^([^\-])/+\1/g'| sed s/"\-"/""/g | sed s/+/"\-"/g | tr '\n' ' ' | sed s/"\-0"/" 0"/g > new_clause.cnf
	
	echo $COUNT
	echo `cat new_clause.cnf` >> thing.cnf
	echo `cat new_clause.cnf` >> $N.models
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
