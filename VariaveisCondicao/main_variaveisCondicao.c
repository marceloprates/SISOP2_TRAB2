#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

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
  pthread_cond_t cond;
 
} garfo_t;

//array de garfos:
garfo_t * garfos;

//variavel global: numero de filosofos e garfos
int numFG = 0;

//variável para impedir que mais de n-1 filósofos comam ao mesmo tempo
pthread_cond_t chairs;
int numChairs = 0;

//mutex para o uso das variáveis de condição
pthread_mutex_t mutex;

//mutex para impedir q as threads se atravessem na impressao do estado na tela
pthread_mutex_t mutexImpressao;

pthread_cond_t filosofosInicializados;
pthread_mutex_t mutexFilosofosInicializados;
int numFilosofosInicializados;

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

  //inicializa a mutex impressão
  pthread_mutex_init(&mutexImpressao,NULL);
  
  //inicializa a variável de condição que determina se todos os filósofos foram inicializados e o seu mutex
  pthread_cond_init(&filosofosInicializados, NULL);
  pthread_mutex_init(&mutexFilosofosInicializados,NULL);

  //inicializa os garfos
  garfos = (garfo_t *) malloc(sizeof(garfo_t) * numFG);
  for (i = 0; i < numFG; i++)
  {
    //garfos[i].estado = 'N';
    pthread_cond_init(&(garfos[i].cond), NULL);
  }

  //inicializa mutex
  pthread_mutex_init(&mutex, NULL);
  
  //inicializa a variável chairs
  numChairs = numFG-1;
  pthread_cond_init(&chairs, NULL);
  
  //inicializa os filosofos
  filosofos = (filosofo_t *) malloc(sizeof(filosofo_t) * numFG);
  numFilosofosInicializados = 0;
  for (i=0; i < numFG; i++)
  {
    filosofos[i].estado = 'I'; //estado inicial I
    numFilosofosInicializados += 1;
    pthread_create(&(filosofos[i].thread), NULL, comportamentoFilosofo, (void*)i);    
  }

  //agora roda pra sempre: usar ctrl+c para parar
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
    pthread_cond_wait(&filosofosInicializados,&mutexFilosofosInicializados);

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
      filosofos[id].estado = 'T';
      imprimeEstado();
      sleep(rand() % 10 + 1); //dorme de 1 a 10 segundos
    }
    //se esta no estado E vai para T e libera os garfos
    else if (filosofos[id].estado == 'E')
    {
        filosofos[id].estado = 'T';
        imprimeEstado();
      
        pthread_mutex_lock(&mutex);
        //libera garfos
        garfos[garfoEsq].estado = 'N';
        pthread_cond_signal(&(garfos[garfoEsq].cond));
        garfos[garfoDir].estado = 'N';
        pthread_cond_signal(&(garfos[garfoDir].cond));
        //libera cadeira
        numChairs++;
        pthread_cond_signal(&chairs);
        
        pthread_mutex_unlock(&mutex);
        
        
        sleep(rand() % 10 + 1); //dorme de 1 a 10 segundos
    }
    
    //se esta no estado T vai para H
    else if (filosofos[id].estado == 'T')
    {
      filosofos[id].estado = 'H';
      imprimeEstado();
        
      //pega cadeira
      pthread_mutex_lock(&mutex);
      while(numChairs == 0)
          pthread_cond_wait(&chairs, &mutex);
      numChairs--;
      pthread_mutex_unlock(&mutex);

      if(id == 0)
      {
        pthread_mutex_lock(&mutex);
        //pega garfos
        while(garfos[garfoDir].estado == 'S')
             pthread_cond_wait(&(garfos[garfoDir].cond), &mutex);
        garfos[garfoDir].estado = 'S';
        while(garfos[garfoEsq].estado == 'S')
             pthread_cond_wait(&(garfos[garfoEsq].cond), &mutex);
        garfos[garfoEsq].estado = 'S';
     
        pthread_mutex_unlock(&mutex);

      }
      else
      { 
        pthread_mutex_lock(&mutex);
        
        while(garfos[garfoEsq].estado == 'S')
            pthread_cond_wait(&(garfos[garfoEsq].cond), &mutex);
        garfos[garfoEsq].estado = 'S';
        while(garfos[garfoDir].estado == 'S')
            pthread_cond_wait(&(garfos[garfoDir].cond), &mutex);
        garfos[garfoDir].estado = 'S';
        
        pthread_mutex_unlock(&mutex);
        
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
