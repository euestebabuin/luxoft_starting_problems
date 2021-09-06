gcc -Wall -c -g main_prob1.c
gcc -Wall -c -g sl_api.c
gcc -o ex_main main_prob1.o sl_api.o -pthread
