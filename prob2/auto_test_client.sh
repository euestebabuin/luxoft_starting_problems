for i in $(seq 1 100)
do
	name=/client_queue
	name+=$i
	./client_ex $name
done
