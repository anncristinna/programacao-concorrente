#include <stdio.h>
#include <stdlib.h> 
#include <pthread.h>

long int soma = 0; //variavel compartilhada entre as threads
pthread_mutex_t mutex; //variavel de lock para exclusao mutua
pthread_cond_t cond; //variavel condicional
short int pendente = 0;
int nthreads_global;
int ativas = 0;

//funcao executada pelas threads
void *ExecutaTarefa (void *arg) {
  long int id = (long int) arg;
  printf("Thread : %ld esta executando...\n", id);

  for (int i=0; i<100000; i++) {
     
     pthread_mutex_lock(&mutex);

     while(pendente == 1) {
        pthread_cond_wait(&cond, &mutex); //enquanto tiver multiplo pendente, esperamos
     }
     
     soma++; 
     if((soma % 1000) == 0 && pendente == 0) {
        pendente = 1;
        pthread_cond_signal(&cond); // como tem coisa pra imprimir, acorda a extra
        while(pendente == 1) {
            pthread_cond_wait(&cond, &mutex); //enquanto tiver multiplo pra imprimir, a thread espera p avançar
        }
     }
    
     pthread_mutex_unlock(&mutex);
  }

  pthread_mutex_lock(&mutex);
  ativas--;
  pthread_cond_broadcast(&cond);
  pthread_mutex_unlock(&mutex);

  printf("Thread : %ld terminou!\n", id);
  pthread_exit(NULL);
}

//funcao executada pela thread de log
void *extra (void *args) {
  printf("Extra : esta executando...\n");
  while(1) {
    pthread_mutex_lock(&mutex);
    while(pendente == 0 && ativas > 0) {
        pthread_cond_wait(&cond, &mutex); //enquanto nao tiver multiplo pra imprimir espera
    }
    if(pendente == 1) {
        printf("soma = %ld\n", soma);
        pendente = 0;
        pthread_cond_broadcast(&cond); //agora q ja imprimiu libera todo mundo
    } else if (ativas == 0) {
        break;
    }
    pthread_mutex_unlock(&mutex);
  }
  printf("Extra : terminou!\n");
  pthread_exit(NULL);
}

//fluxo principal
int main(int argc, char *argv[]) {
   pthread_t *tid; //identificadores das threads no sistema

   //--le e avalia os parametros de entrada
   if(argc<2) {
      printf("Digite: %s <numero de threads>\n", argv[0]);
      return 1;
   }
   nthreads_global = atoi(argv[1]);
   ativas = nthreads_global;

   //--aloca as estruturas
   tid = (pthread_t*) malloc(sizeof(pthread_t)*(nthreads_global+1));
   if(tid==NULL) {puts("ERRO--malloc"); return 2;}

   //--inicilaiza o mutex (lock de exclusao mutua)
   pthread_mutex_init(&mutex, NULL);
   pthread_cond_init (&cond, NULL);

   //--cria as threads
   for(long int t=0; t<nthreads_global; t++) {
     if (pthread_create(&tid[t], NULL, ExecutaTarefa, (void *)t)) {
       printf("--ERRO: pthread_create()\n"); exit(-1);
     }
   }

   //--cria thread de log
   if (pthread_create(&tid[nthreads_global], NULL, extra, NULL)) {
      printf("--ERRO: pthread_create()\n"); exit(-1);
   }

   //--espera todas as threads terminarem
   for (int t=0; t<nthreads_global+1; t++) {
     if (pthread_join(tid[t], NULL)) {
         printf("--ERRO: pthread_join() \n"); exit(-1); 
     } 
   } 

   //--finaliza o mutex
   pthread_mutex_destroy(&mutex);
   pthread_cond_destroy(&cond);
   
   printf("Valor de 'soma' = %ld\n", soma);

   return 0;
}