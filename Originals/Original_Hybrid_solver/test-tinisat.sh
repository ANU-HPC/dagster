rm tinisat-uf250-results
ls Benchmarks/uf250-1065 | while read line
do
	echo "Testing $line"
	./Originals/tinisat0.22/tinisat Benchmarks/uf250-1065/$line >> tinisat-uf250-results
done

rm tinisat-uuf250-results
ls Benchmarks/uuf250-1065 | while read line
do
	echo "Testing $line"
	./Originals/tinisat0.22/tinisat Benchmarks/uuf250-1065/$line >> tinisat-uuf250-results
done

rm tinisat-random-3sat-v360-results
ls Benchmarks/random/OnTreshold/3SAT/v360| grep "\.SAT\." | while read line
do
	echo "Testing $line"
	timeout 5m ./Originals/tinisat0.22/tinisat Benchmarks/random/OnTreshold/3SAT/v360/$line >> tinisat-random-3sat-v360-results

done

rm tinisat-random-3sat-v400-results
ls Benchmarks/random/OnTreshold/3SAT/v400| grep "\.SAT\." | while read line
do
	echo "Testing $line"
	timeout 5m ./Originals/tinisat0.22/tinisat Benchmarks/random/OnTreshold/3SAT/v400/$line >> tinisat-random-3sat-v400-results

done
