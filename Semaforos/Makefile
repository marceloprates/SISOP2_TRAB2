target: jantar_semaforos

jantar_semaforos: main_semaforos.o
	gcc -o jantar_semaforos -pthread main_semaforos.o 

main_semaforos.o: main_semaforos.c
	gcc -c main_semaforos.c

clean:
	rm -f *.o jantar_semaforos
