#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void geraVetores(float *a, float *b, long N) {
    srand(time(NULL));

    for(long i = 0; i < N; i++) {
        a[i] = (float) rand() / RAND_MAX;
        b[i] = (float) rand() / RAND_MAX;
    }
}

float calculaProdutoInterno(float *a, float *b, long N) {
    float soma = 0;

    for(long i = 0; i < N; i++) {
        soma += a[i] * b[i];
    }

    return soma;
}

void salvaArquivo(const char *nomeArquivo, long N, float *a, float *b, float resultado) {
    FILE *f = fopen(nomeArquivo, "wb"); //abre o arquivo para escrita binaria
    if(!f) {
        printf("Erro ao abrir arquivo paar escrita binária");
        exit(1);
    }
    
    fwrite(&N, sizeof(long), 1, f);
    fwrite(a, sizeof(float), N, f);
    fwrite(b, sizeof(float), N, f);
    fwrite(&resultado, sizeof(float), 1, f);

    fclose(f);

}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Passe a dimensão do vetor");
        return 1;
    }

    long N;
    N = atol(argv[1]);

    float *a = malloc(sizeof(float) * N);
    float *b = malloc(sizeof(float) * N);
    if(!a || !b) {
        printf("Erro ao alocar memória");
        return 1;
    }

    geraVetores(a, b, N);
    float resultado = calculaProdutoInterno(a, b, N);

    const char *arquivo = "dados.bin";
    salvaArquivo(arquivo, N, a, b, resultado);

    free(a);
    free(b);

    return 0;
}
