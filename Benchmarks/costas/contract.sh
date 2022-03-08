#!/bin/bash

v1=$1
v2=$2
file=$3

NODES=`cat $file | grep NODES | awk -F":" '{print $2}'`
NODES=$(($NODES - 1))

v1varsuffix=`cat $file | grep "\->" | grep ^${v1} | awk -F":" '{print $2}'`
v2varsuffix=`cat $file | grep "\->" | grep ^${v2} | awk -F":" '{print $2}'`

v1clausuffix=`cat $file | grep -v "\->" | grep "\-" | grep ":" | grep ^${v1} | awk -F":" '{print $2}'`
v2clausuffix=`cat $file | grep -v "\->" | grep "\-" | grep ":" | grep ^${v2} | awk -F":" '{print $2}'`

while read p ;
do
    if echo "$p" | grep -q NODES ; then
	echo "NODES:$NODES"
	continue
    fi

    if echo "$p" | grep "\->" |  grep -q ^${v1} ; then
	PREFIX=`echo "$p" | awk -F":" '{print $1}'`
	echo "${PREFIX}:${v1varsuffix},$v2varsuffix"
	continue
    fi

    
    if echo "$p" | grep  "\->" | grep -q ^${v2} ; then
	continue;
    fi

    if echo "$p" | grep -q "\->" ; then
	PREFIX=`echo "$p" | awk -F"->" '{print $1}'`
	SUFFIX=`echo "$p" | awk -F":" '{print $2}'`
	NUM=`echo "$p" | awk -F"->" '{print $2}' | awk -F":" '{print $1}'`
	NUM=$(($NUM - 1))
	PREFIX=$(($PREFIX - 1))
	echo "${PREFIX}->${NUM}:${SUFFIX}"
	continue
    fi

    if echo "$p" | grep  "\-" | grep  ":" | grep -q ^${v1} ; then
	echo "${v1}:${v2clausuffix}"
	continue
    fi

    if echo "$p" | grep  "\-" | grep  ":" | grep -q ^${v2} ; then
	continue
    fi

    
    if echo "$p" | grep  "\-" | grep -q ":" ; then
	PREFIX=`echo "$p" | awk -F":" '{print $1}'`
	SUFFIX=`echo "$p" | awk -F":" '{print $2}'`
	PREFIX=$(($PREFIX - 1))
	echo "${PREFIX}:${SUFFIX}"
	continue
    fi
 
    
    echo "$p"
done < $file
