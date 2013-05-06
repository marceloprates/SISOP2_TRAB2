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

//mutex para impedir q as threads se atravessem na impressao do estado na tela
pthread_mutex_t mutexImpressao;

pthread_cond_t filosofosInicializados;
pthread_mutex_t mutexFilosofosInicializados;
int numFilosofosInicializados;

//funções:
void * trocaEstadoFilosofo(int idFilosofo, char novoEstado);
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

  //inicializa a mutex impressão
  pthread_mutex_init(&mutexImpressao,NULL);
  
  //inicializa os garfos
  garfos = (garfo_t *) malloc(sizeof(garfo_t) * numFG);
  for (i = 0; i < numFG; i++)
    sem_init(&(garfos[i].semaphore), 0, 1); //inicializa em 1 pois somente um filosofo podera usar o garfo por vez
  
  //inicializa os filosofos
  filosofos = (filosofo_t *) malloc(sizeof(filosofo_t) * numFG);
  numFilosofosInicializados = 0;
  for (i=0; i < numFG; i++)
  {
    filosofos[i].estado = 'I'; //estado inicial I
    numFilosofosInicializados += 1;
    pthread_create(&(filosofos[i].thread), NULL, comportamentoFilosofo, (void*)i ); 
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
  pthread_mutex_lock(&mutexFilosofosInicializados);

  while(numFilosofosInicializados < numFG)
    pthread_cond_wait(&filosofosInicializados, &mutexFilosofosInicializados);

  pthread_cond_signal(&filosofosInicializados);

  pthread_mutex_unlock(&mutexFilosofosInicializados);

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
      trocaEstadoFilosofo(id, 'T');
      sleep(rand() % 10 + 1); //dorme de 1 a 10 segundos
    }
    //se esta no estado E vai para T e libera os garfos
    else if (filosofos[id].estado == 'E')
    {
      trocaEstadoFilosofo(id, 'T');
      sem_post(&(garfos[garfoEsq].semaphore));
      sem_post(&(garfos[garfoDir].semaphore));
      sleep(rand() % 10 + 1); //dorme de 1 a 10 segundos
    }
    
    //se esta no estado T vai para H
    else if (filosofos[id].estado == 'T')
    {
      trocaEstadoFilosofo(id, 'H');

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
      trocaEstadoFilosofo(id, 'E');
      sleep(rand() % 10 + 1); //dorme de 1 a 10 segundos
    }      
  }  
}

//troca o estado do filoso para o novo estado, em seção crítica para impedir erros de impressao
//de estados inconsistentes
void * trocaEstadoFilosofo(int idFilosofo, char novoEstado)
{
  int i;
  
  pthread_mutex_lock(&mutexImpressao);
  filosofos[idFilosofo].estado = novoEstado;
  for (i = 0; i < numFG; i++)
    fprintf(stderr, "%c ", filosofos[i].estado);
  
  fprintf(stderr, "\n");
  pthread_mutex_unlock(&mutexImpressao);
}