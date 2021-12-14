ssu_convert: ssu_convert.o ssu_token.o
	gcc -g -o ssu_convert ssu_convert.o ssu_token.o 

ssu_convert.o: ssu_extern.h ssu_token.h ssu_convert.c
	gcc -c -o ssu_convert.o ssu_convert.c

ssu_token.o : ssu_token.h ssu_token.c
	gcc -c -o ssu_token.o ssu_token.c
