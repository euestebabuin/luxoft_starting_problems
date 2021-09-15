#server program must be running

for i in $(seq 1 10)
do
	folder_name=test
	large_name=large
	folder_name+=$i
	large_name+=$i
	./generate_large.sh > $large_name
	mkdir $folder_name
	cp ./client_ex ./$folder_name/client_ex
	cd $folder_name
	./client_ex 12345 ./$large_name &
	cd ..
done
