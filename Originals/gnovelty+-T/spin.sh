#!/bin/bash

# This is a script C. Gretton wrote that can be used to sanity check the Dagster results.
# It is a very primitive model enumeration procedure based on gNovelty+. 

if [ "$#" -ne 1 && "$#" -ne 3 ]
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

cp thing.cnf original.cnf



## Do we need to take a projection of the original problem?
if [ "$#" -eq 3 ]
then
    OLDNUMCLAUSES=`cat original.cnf | grep ^p | awk '{print $4}'`
    OLDNUMVARS=`cat original.cnf | grep ^p | awk '{print $3}'`
    
    targetrange="$2"   ## Range of variables that we are counting solutions over

    
    
    if [ -f target.cnf ] ; then
	echo "NOTE: deleting the file target.cnf" >&2
	rm target.cnf
    fi

    
    if [ -f _target.cnf ] ; then
	echo "NOTE: deleting the file _target.cnf" >&2
	rm _target.cnf
    fi

    touch target.cnf
    
    
    for arange in `echo "$3" | tr ',' '\n'` ; do  
	targetproblem="$arange" ## Range of clauses that we are taking from 'original.cnf'

	fromfrom=`echo "$targetproblem" | tr '-' '\n' | head -1`
	fromfrom=`echo "print( $fromfrom + 1 )" | python`
	readuntil=`echo "$targetproblem" | tr '-' '\n' | tail -1`
	readuntil=`echo "print( $readuntil + 1 )" | python`
	
	cat original.cnf | grep -v ^p > _target.cnf    
    
	sizeoffile=`wc -l _target.cnf | awk '{print $1}'`
    
	echo "Stripping some of the $sizeoffile clauses from _target.cnf"
	if [ $sizeoffile -ne $OLDNUMCLAUSES ] ; then
	    echo "ERROR - More lines than declared clauses"
	    echo "ERROR - Claimed : $OLDNUMCLAUSES"
	    echo "ERROR - Actual lines : $sizeoffile"
	    exit -1
	fi

	SCANHEADLENGTH=`echo "print( $readuntil )"  |  python`
	head -$SCANHEADLENGTH _target.cnf > tt.cnf
	mv tt.cnf _target.cnf
	echo "$SCANHEADLENGTH clauses left in _target.cnf"

	sizeoffile=`wc -l _target.cnf | awk '{print $1}'`
	SCANTAILLENGTH=`echo "print( $sizeoffile - $fromfrom + 1 )"  |  python`
	tail -$SCANTAILLENGTH _target.cnf > tt.cnf
	mv tt.cnf _target.cnf

	cat _target.cnf >> target.cnf
    done
	
    NUMCLAUSES=`wc -l target.cnf | awk '{print $1}'`
    echo "p cnf $OLDNUMVARS $NUMCLAUSES" > original.cnf
    cat target.cnf >> original.cnf
    echo "$NUMCLAUSES clauses left in target.cnf"
    cp original.cnf thing.cnf

    if echo "$targetrange" | grep -q \-  ; then 
	
	smallest=`echo "$targetrange" | tr '-' '\n' | head -1`
	largest=`echo "$targetrange" | tr '-' '\n' | tail -1`
	echo "ITEMS={x for x in range(-$largest,-$smallest + 1)}.union({x for x in range($smallest,$largest + 1)}.union({0}))" > p.py
	echo "def getlines():" >> p.py
	echo "   while True:" >> p.py
	echo "      yield input()" >> p.py
	echo "try:" >> p.py
	echo "   for x in getlines():" >> p.py
	echo "      if int(x) in ITEMS: print(x)" >> p.py
	echo "except EOFError as e:" >> p.py
	echo "   pass" >> p.py
    else
	echo "ITEMS={$targetrange}.union({0})" > p.py
	echo "def getlines():" >> p.py
	echo "   while True:" >> p.py
	echo "      yield input()" >> p.py
	echo "try:" >> p.py
	echo "   for x in getlines():" >> p.py
	echo "      if int(x) in ITEMS: print(x)" >> p.py
	echo "except EOFError as e:" >> p.py
	echo "   pass" >> p.py
    fi
    
    rm target.cnf ## CLEANUP
else
    echo "def getlines():" > p.py
    echo "   while True:" >> p.py
    echo "      yield input()" >> p.py
    echo "try:" >> p.py
    echo "   for x in getlines():" >> p.py
    echo "      print(x)" >> p.py
    echo "except EOFError as e:" >> p.py
    echo "   pass" >> p.py
fi


if [ -f costas.$MODELS_ANNOTATION.models ] ; then
    rm  costas.$MODELS_ANNOTATION.models
fi

touch costas.$MODELS_ANNOTATION.models

#exit -9

COUNT=0

while true ; do
    #DEBUG#wc -l thing.cnf
    #DEBUG#head -1 thing.cnf
    for f in {1..16} ; do 
	./gnovelty+ thing.cnf > OUTPUT$f 2> ERROR$f &
    done
    while true ; do
	X=`ps xg | grep gnovelty+ | wc -l`
	if [ $X -eq 1 ] ; then
	    break
	fi
	sleep .05
    done

    if [ -f new_clause.cnf ] ; then
	rm new_clause.cnf
    fi
    touch new_clause.cnf
    
    for f in {1..16} ; do
	cp OUTPUT$f OUTPUT
	
	if cat OUTPUT | grep -q "s SATISFIABLE" ; then 
	    cat OUTPUT | grep -v "Time " | grep 0$ | tr ' ' '\n' | python3 p.py | sed -E 's/^([^\-])/+\1/g'| sed s/"\-"/""/g | sed s/+/"\-"/g | tr '\n' ' ' | sed s/"\-0"/" 0"/g >> new_clause.cnf
	    
	    

	    cat new_clause.cnf | sort -u | wc -l 
	    wc -l new_clause.cnf

	    echo "" >> new_clause.cnf
	    cat  new_clause.cnf
	    COUNT=$(($COUNT + 1))
	else
	    break;
	fi
    done


    OLDNUMCLAUSES=`cat original.cnf | grep ^p | awk '{print $4}'`
    cat new_clause.cnf | sort -u >> costas.$MODELS_ANNOTATION.models
    cat costas.$MODELS_ANNOTATION.models | sort -u > TTT
    cp costas.$MODELS_ANNOTATION.models TTT
    rm TTT
    CLAUSESINCREMENT=`cat costas.$MODELS_ANNOTATION.models | wc -l`
    NEWNUMCLAUSES=$(($OLDNUMCLAUSES + $CLAUSESINCREMENT))
    cat original.cnf | sed s/"$OLDNUMCLAUSES"/"$NEWNUMCLAUSES"/g > tmp.cnf
    cat costas.$MODELS_ANNOTATION.models >> tmp.cnf
    mv tmp.cnf thing.cnf
    
done

echo "Number of models found for coastas $N is: $COUNT"
echo "These have been stored is the 'nogood' clauses at: costas.$MODELS_ANNOTATION.models"
wc -l costas.$MODELS_ANNOTATION.models
