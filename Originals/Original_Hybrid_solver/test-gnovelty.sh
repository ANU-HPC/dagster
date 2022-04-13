rm gnovelty-uf250-results-*
ls Benchmarks/uf250-1065 | while read line
do
	echo "Testing $line"
	for i in {1..10}
	do
		echo $line >> gnovelty-uf250-results-$i
		./Originals/gnovelty+-T/gnovelty+ Benchmarks/uf250-1065/$line >> gnovelty-uf250-results-$i
	done
done

rm gnovelty-random-3sat-v360-results-*
ls Benchmarks/random/OnTreshold/3SAT/v360| grep "\.SAT\." | while read line
do
	echo "Testing $line"
	for i in {1..10}
	do
		echo $line >> gnovelty-random-3sat-v360-results-$i
		./Originals/gnovelty+-T/gnovelty+ Benchmarks/random/OnTreshold/3SAT/v360/$line >> gnovelty-random-3sat-v360-results-$i
	done
done

rm gnovelty-random-3sat-v400-results-*
ls Benchmarks/random/OnTreshold/3SAT/v400| grep "\.SAT\." | while read line
do
	echo "Testing $line"
	for i in {1..10}
	do
		echo $line >> gnovelty-random-3sat-v400-results-$i
		./Originals/gnovelty+-T/gnovelty+ Benchmarks/random/OnTreshold/3SAT/v400/$line >> gnovelty-random-3sat-v400-results-$i
	done
done

rm gnovelty-random-3sat-v450-results-*
ls Benchmarks/random/OnTreshold/3SAT/v450| grep "\.SAT\." | while read line
do
	echo "Testing $line"
	for i in {1..10}
	do
		echo $line >> gnovelty-random-3sat-v450-results-$i
		./Originals/gnovelty+-T/gnovelty+ Benchmarks/random/OnTreshold/3SAT/v450/$line >> gnovelty-random-3sat-v450-results-$i
	done
done

rm gnovelty-random-3sat-v500-results-*
ls Benchmarks/random/OnTreshold/3SAT/v500| grep "\.SAT\." | while read line
do
	echo "Testing $line"
	for i in {1..10}
	do
		echo $line >> gnovelty-random-3sat-v500-results-$i
		./Originals/gnovelty+-T/gnovelty+ Benchmarks/random/OnTreshold/3SAT/v500/$line >> gnovelty-random-3sat-v500-results-$i
	done
done

rm gnovelty-random-3sat-v550-results-*
ls Benchmarks/random/OnTreshold/3SAT/v550 | grep "\.SAT\." | while read line
do
	echo "Testing $line"
	for i in {1..10}
	do
		echo $line >> gnovelty-random-3sat-v550-results-$i
		./Originals/gnovelty+-T/gnovelty+ Benchmarks/random/OnTreshold/3SAT/v550/$line >> gnovelty-random-3sat-v550-results-$i
	done
done

rm gnovelty-random-3sat-v600-results-*
ls Benchmarks/random/OnTreshold/3SAT/v600| grep "\.SAT\." | while read line
do
	echo "Testing $line"
	for i in {1..10}
	do
		echo $line >> gnovelty-random-3sat-v600-results-$i
		./Originals/gnovelty+-T/gnovelty+ Benchmarks/random/OnTreshold/3SAT/v600/$line >> gnovelty-random-3sat-v600-results-$i
	done
done

rm gnovelty-random-3sat-v650-results-*
ls Benchmarks/random/OnTreshold/3SAT/v650| grep "\.SAT\." | while read line
do
	echo "Testing $line"
	for i in {1..10}
	do
		echo $line >> gnovelty-random-3sat-v650-results-$i
		./Originals/gnovelty+-T/gnovelty+ Benchmarks/random/OnTreshold/3SAT/v650/$line >> gnovelty-random-3sat-v650-results-$i
	done
done

rm gnovelty-random-3sat-v4000-results-*
ls Benchmarks/random/LargeSize/3SAT/v4000 |  while read line
do
	echo "Testing $line"
	for i in {1..10}
	do
		echo $line >> gnovelty-random-3sat-v4000-results-$i
		./Originals/gnovelty+-T/gnovelty+ Benchmarks/random/LargeSize/3SAT/v4000/$line >> gnovelty-random-3sat-v4000-results-$i
	done
done
