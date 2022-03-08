FILEINPUT=$1
numvars=`head -1 $FILEINPUT | awk '{print $3}'`
numclause=`head -1 $FILEINPUT | awk '{print $4}'`
echo "DAG-FILE" > `echo "$FILEINPUT" | sed s/"\.cnf"/"\.dag"/g`
echo "NODES:1" >> `echo "$FILEINPUT" | sed s/"\.cnf"/"\.dag"/g`
echo "GRAPH:" >> `echo "$FILEINPUT" | sed s/"\.cnf"/"\.dag"/g`
echo "CLAUSES:" >> `echo "$FILEINPUT" | sed s/"\.cnf"/"\.dag"/g`
echo "0:0-$(($numclause - 1))" >> `echo "$FILEINPUT" | sed s/"\.cnf"/"\.dag"/g`
echo "REPORTING:" >> `echo "$FILEINPUT" | sed s/"\.cnf"/"\.dag"/g`
echo "1-$numvars" >> `echo "$FILEINPUT" | sed s/"\.cnf"/"\.dag"/g`
