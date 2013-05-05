#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
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
  pthread_cond_t cond_var;
  pthread_mutex_t mutex;
 
} garfo_t;

//array de garfos:
garfo_t * garfos;

//variavel global: numero de filosofos e garfos
int numFG = 0;

//mutex para impedir q as threads se atravessem na impressao do estado na tela
pthread_mutex_t mutexImpressao;//PTHREAD_MUTEX_INITIALIZER;

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

  //inicializa mutex da impressão
  pthread_mutex_init(&mutexImpressao,NULL);
  
  //inicializa os garfos
  garfos = (garfo_t *) malloc(sizeof(garfo_t) * numFG);
  for (i = 0; i < numFG; i++)
  {
  	garfos[i].estado = 'S';
    pthread_cond_init(&(garfos[i].cond_var),NULL);
    pthread_mutex_init(&(garfos[i].mutex),NULL);
  }
  
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

      pthread_mutex_lock(&(garfos[garfoEsq].mutex));
      pthread_cond_signal(&(garfos[garfoEsq].cond_var));
      garfos[garfoEsq].estado = 'S';
      pthread_mutex_unlock(&(garfos[garfoEsq].mutex));

      pthread_mutex_lock(&(garfos[garfoDir].mutex));
      pthread_cond_signal(&(garfos[garfoDir].cond_var));
      garfos[garfoDir].estado = 'S';
      pthread_mutex_unlock(&(garfos[garfoDir].mutex));

      sleep(rand() % 10 + 1); //dorme de 1 a 10 segundos
    }
    
    //se esta no estado T vai para H
    else if (filosofos[id].estado == 'T')
    {
      filosofos[id].estado = 'H';
      imprimeEstado();

      if(id == 0)
      {
      	pthread_mutex_lock(&(garfos[garfoDir].mutex));
      	while(garfos[garfoDir].estado == 'N')
        	pthread_cond_wait(&(garfos[garfoDir].cond_var),&(garfos[garfoDir].mutex));
        garfos[garfoDir].estado = 'N';
        pthread_mutex_unlock(&(garfos[garfoDir].mutex));

        pthread_mutex_lock(&(garfos[garfoEsq].mutex));
        while(garfos[garfoEsq].estado == 'N')
        	pthread_cond_wait(&(garfos[garfoEsq].cond_var),&(garfos[garfoEsq].mutex));
        garfos[garfoEsq].estado = 'N';
        pthread_mutex_unlock(&(garfos[garfoEsq].mutex));
      	
      }
      else
      {
      	pthread_mutex_lock(&(garfos[garfoEsq].mutex));
        while(garfos[garfoEsq].estado == 'N')
        	pthread_cond_wait(&(garfos[garfoEsq].cond_var),&(garfos[garfoEsq].mutex));
        garfos[garfoEsq].estado = 'N';
        pthread_mutex_unlock(&(garfos[garfoEsq].mutex));

        pthread_mutex_lock(&(garfos[garfoDir].mutex));
      	while(garfos[garfoDir].estado == 'N')
        	pthread_cond_wait(&(garfos[garfoDir].cond_var),&(garfos[garfoDir].mutex));
        garfos[garfoDir].estado = 'N';
        pthread_mutex_unlock(&(garfos[garfoDir].mutex));
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