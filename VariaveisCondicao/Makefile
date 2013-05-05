target: jantar_variaveisCondicao

jantar_variaveisCondicao: main_variaveisCondicao.o
	gcc -o jantar_variaveisCondicao -pthread main_variaveisCondicao.o 

main_variaveisCondicao.o: main_variaveisCondicao.c
	gcc -c main_variaveisCondicao.c

clean:
	rm -f *.o jantar_variaveisCondicao
