#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>

float *a;
float *b;

typedef struct {
    short int id;
    short int n_threads;
    long int N;
} Args;

void *produtoParcial(void *arg) {
    Args *args = (Args * ) arg;
    long int fatia, ini, fim;
    float soma = 0;
    
    fatia = args -> N / args -> n_threads;
    ini = args -> id * fatia;
    fim = (args->id == args->n_threads - 1) ? args->N : ini + fatia;

    for(long int i = ini; i < fim; i++) {
        soma += a[i] * b[i];
    }

    float *res = malloc(sizeof(float));
    *res = soma;

    free(args);

    pthread_exit(res);
}


int main(int argc, char *argv[]) {
    //delcarao de variaveis
    short int nthreads;
    long int N;
    float total;
    float total_conc = 0.0, total_seq = 0.0, valor_arquivo = 0.0;
    FILE *fp;

    //verificacao de entrada
    if(argc < 3) {
        printf("Devem ser passados o nome do arquivo e o número de threads");
        return 1;
    }

    //abre arquivo 
    fp = fopen(argv[1], "rb"); //abre para leitura de binario
    if(!fp){
        printf("Erro ao abrir arquivo para leitura");
        return 1;
    }
    fread(&N, sizeof(long), 1, fp);// le a dimensao N

    a = malloc(sizeof(float) * N);
    b  = malloc(sizeof(float) * N);
    if(!a || !b) {
        printf("Erro ao alocar memória.");
        return 1;
    }

    //le os vetores do arquivo
    fread(a, sizeof(float), N, fp);
    fread(b, sizeof(float), N, fp);

    fread(&valor_arquivo, sizeof(float), 1, fp);

    fclose(fp);


    //inicializacao das variaveis
    nthreads = atoi(argv[2]);
    pthread_t tid[nthreads];

    clock_t inicio = clock();

    //cria as threads
    for(short int i = 0; i < nthreads; i++) {
        Args *args = malloc(sizeof(Args));
        if(args == NULL) {
            printf("Erro ao alocar argumentos");
            return 1;
        }
        args -> id = i;
        args -> n_threads = nthreads;
        args -> N = N;
        pthread_create(&tid[i], NULL, produtoParcial, (void *) args);
    }

    //espera todas as threads terminarem e pega o valor retornado por elas
    for(short int i = 0; i < nthreads; i++) {
        float *retorno;
        if(pthread_join(tid[i], (void **)&retorno)){
            printf("Erro");
        }
        total_conc += *retorno;
        free(retorno);
    }

    clock_t fim = clock();
    double tempo_execucao = (double) (fim - inicio) / CLOCKS_PER_SEC;

    for(long int i = 0; i < N; i++) {
        total_seq += a[i] * b[i];
    }

    float e = (total_seq - total_conc) / total_seq;

    printf("Produto interno concorrente = %.6f\n", total_conc);
    printf("Produto interno sequencial = %.6f\n", total_seq);
    printf("Variação relativa = %.6f", e);
    printf("Tempo de execução concorrente = %.6fs\n", tempo_execucao);
    
    free(a);
    free(b);

    return 0;
}