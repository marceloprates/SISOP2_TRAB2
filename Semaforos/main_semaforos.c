#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <semaphore.h>
#include <unistd.h>

typedef struct FILOSOFO_T
{
  char estado; //T, H ou E
  pthread_t thread;
   
} filosofo_t;

//array de filosofos
filosofo_t * filosofos;

typedef struct GARFO_T
{
  char estado; //em: S, ao contrario: N
  sem_t semaphore;
 
} garfo_t;

//array de garfos:
garfo_t * garfos;

//variavel global: numero de filosofos e garfos
int numFG = 0;

//semáforo para impedir que mais de n-1 filósofos comam ao mesmo tempo
sem_t chairs;

//mutex para impedir q as threads se atravessem na impressao do estado na tela
pthread_mutex_t mutexImpressao = PTHREAD_MUTEX_INITIALIZER;

//funções:
void imprimeEstado(void);
void * comportamentoFilosofo(void * arg);


int main (int argc, char ** argv)
{
  int i;
  //pega argumentos
  if (argc != 2)
  {
    fprintf(stderr, "\nUso: ./jantar_semaforos n \nn: numero de filosofos e garfos.\n");
    return 0;
  }
  
  numFG = atoi(argv[1]);
  if (numFG == 0)
  {
    fprintf(stderr, "\nNumero de filosofos e garfos invalido. Uso: ./jantar_semaforos n \nn: numero de filosofos e garfos.\n");
    return 0;
  }
  
  //inicializa os garfos
  garfos = (garfo_t *) malloc(sizeof(garfo_t) * numFG);
  for (i = 0; i < numFG; i++)
    sem_init(&(garfos[i].semaphore), 0, 1); //inicializa em 1 pois somente um filosofo podera usar o garfo por vez

  //inicializa o semáforo chairs
  sem_init(&chairs,0,numFG-1);
  
  //inicializa os filosofos
  filosofos = (filosofo_t *) malloc(sizeof(filosofo_t) * numFG);
  for (i=0; i < numFG; i++)
  {
    filosofos[i].estado = 'I'; //estado inicial I
    pthread_create(&(filosofos[i].thread), NULL, comportamentoFilosofo, i ); 
  }
  
  //agora roda pra sempre: usar ctrl+c
  while (1)
  {
    sleep(100);
  } 
}

//recebe como parametro o id do filosofo
void * comportamentoFilosofo(void * arg)
{
  int id = (int) arg;
  
  int garfoEsq, garfoDir;
  
  //configuracao: garfo0,filosofo0,g1,f1,g2,f2 etc
  garfoEsq = id;
  garfoDir = (id + 1) % numFG;
  
  while(1)
  {
    //se está no estado inicial, passa para T
    if (filosofos[id].estado == 'I')
    {
      filosofos[id].estado = 'T';
      imprimeEstado();
      sleep(rand() % 10 + 1); //dorme de 1 a 10 segundos
    }
    //se esta no estado E vai para T e libera os garfos
    else if (filosofos[id].estado == 'E')
    {
      filosofos[id].estado = 'T';
      imprimeEstado();
      sem_post(&(garfos[garfoEsq].semaphore));
      sem_post(&(garfos[garfoDir].semaphore));
      sem_post(&chairs);
      sleep(rand() % 10 + 1); //dorme de 1 a 10 segundos
    }
    
    //se esta no estado T vai para H
    else if (filosofos[id].estado == 'T')
    {
      filosofos[id].estado = 'H';
      imprimeEstado();

      sem_wait(&chairs);

      if(id == 0)
      {
        sem_wait(&(garfos[garfoDir].semaphore));
        sem_wait(&(garfos[garfoEsq].semaphore));
      }
      else
      { 
        sem_wait(&(garfos[garfoEsq].semaphore));
        sem_wait(&(garfos[garfoDir].semaphore));
      }
    }
    
    //se esta no estado H vai para E
    else if (filosofos[id].estado == 'H')
    {
      filosofos[id].estado = 'E';
      imprimeEstado();
      sleep(rand() % 10 + 1); //dorme de 1 a 10 segundos
    }      
  }  
}

void imprimeEstado(void)
{
  int i;
  
  pthread_mutex_lock(&mutexImpressao);
  for (i = 0; i < numFG; i++)
    fprintf(stderr, "%c ", filosofos[i].estado);
  
  fprintf(stderr, "\n");
  pthread_mutex_unlock(&mutexImpressao);
}