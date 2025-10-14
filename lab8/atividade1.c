#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <math.h>

int *canal;
int N, M;
int in = 0, out = 0;
int consumidores;
int count_primos = 0;
int *primos_thread;
int producao_terminou = 0;

sem_t mutex;
sem_t cheio;
sem_t vazio;
sem_t produtor_ok;

int ehPrimo(long long int n) {
    if (n <= 1) return 0;
    if (n == 2) return 1;
    if (n % 2 == 0) return 0;
    for (long long int i = 3; i <= sqrt(n); i += 2)
        if (n % i == 0) return 0;
    return 1;
}

void *produtor(void *arg) {
    int i = 1;

    while (i <= N) {
        sem_wait(&produtor_ok);

        for (int j = 0; j < M && i <= N; j++, i++) {
            sem_wait(&vazio);  
            sem_wait(&mutex);

            canal[in] = i;
            in = (in + 1) % M;

            sem_post(&mutex);
            sem_post(&cheio);  
        }
    }

    producao_terminou = 1;

    for (int i = 0; i < consumidores; i++)
        sem_post(&cheio);  

    pthread_exit(NULL);
}

void *consumidor(void *arg) {
    int id = *(int *)arg;
    int local_primos = 0;

    while (1) {
        sem_wait(&cheio); 

        if (producao_terminou && sem_trywait(&mutex) == 0) {
            sem_post(&mutex);
            break;
        }

        sem_wait(&mutex);
        int num = canal[out];
        out = (out + 1) % M;
        int buffer_vazio = (in == out);  
        sem_post(&mutex);

        sem_post(&vazio);  

        if (buffer_vazio)
            sem_post(&produtor_ok);  

        if (ehPrimo(num))
            local_primos++;
    }

    primos_thread[id] = local_primos;

    __sync_fetch_and_add(&count_primos, local_primos);

    pthread_exit(NULL);
}

int main() {
    printf("Digite a quantidade de números a gerar: ");
    scanf("%d", &N);

    printf("Digite o tamanho do buffer: ");
    scanf("%d", &M);

    printf("Digite o número de threads consumidoras: ");
    scanf("%d", &consumidores);

    canal = malloc(M * sizeof(int));
    primos_thread = calloc(consumidores, sizeof(int));

    sem_init(&mutex, 0, 1);
    sem_init(&cheio, 0, 0);
    sem_init(&vazio, 0, M);
    sem_init(&produtor_ok, 0, 1);  

    pthread_t t_produtor;
    pthread_t t_consumidores[consumidores];
    int ids[consumidores];

    pthread_create(&t_produtor, NULL, produtor, NULL);

    for (int i = 0; i < consumidores; i++) {
        ids[i] = i;
        pthread_create(&t_consumidores[i], NULL, consumidor, &ids[i]);
    }

    pthread_join(t_produtor, NULL);
    for (int i = 0; i < consumidores; i++)
        pthread_join(t_consumidores[i], NULL);

    int vencedor = 0;
    for (int i = 1; i < consumidores; i++)
        if (primos_thread[i] > primos_thread[vencedor])
            vencedor = i;

    printf("\nTotal de números primos encontrados: %d\n", count_primos);
    printf("Thread vencedora: %d (encontrou %d primos)\n",
           vencedor, primos_thread[vencedor]);

    free(canal);
    free(primos_thread);
    sem_destroy(&mutex);
    sem_destroy(&cheio);
    sem_destroy(&vazio);
    sem_destroy(&produtor_ok);

    return 0;
}
