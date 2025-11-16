/* Disciplina: Programacao Concorrente */
/* Prof.: Silvana Rossetto */
/* Laboratório: 11 */
/* Codigo: Exemplo de uso de futures - versão adaptada para contagem de primos */
/* -------------------------------------------------------------------*/

import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;

import java.util.ArrayList;
import java.util.List;

// ------------------------------------------------------------
// Função auxiliar para verificar se um número é primo
class PrimoUtils {
    public static boolean ehPrimo(long n) {
        if (n <= 1) return false;
        if (n == 2) return true;
        if (n % 2 == 0) return false;

        for (long i = 3; i * i <= n; i += 2) {
            if (n % i == 0) return false;
        }
        return true;
    }
}

// ------------------------------------------------------------
// Tarefa Callable para contar primos em um intervalo
class PrimoCallable implements Callable<Long> {
    private final long inicio;
    private final long fim;

    public PrimoCallable(long inicio, long fim) {
        this.inicio = inicio;
        this.fim = fim;
    }

    @Override
    public Long call() {
        long count = 0;
        for (long i = inicio; i <= fim; i++) {
            if (PrimoUtils.ehPrimo(i)) {
                count++;
            }
        }
        return count;
    }
}

// ------------------------------------------------------------
// Classe principal
public class FutureHello {
    private static final long N = 1_000_000;   // pode colocar um N bem grande
    private static final int NTHREADS = 10;    // tamanho do pool
    private static final int NTAREFAS = 20;    // dividir N em 20 pedaços, por exemplo

    public static void main(String[] args) {

        ExecutorService executor = Executors.newFixedThreadPool(NTHREADS);
        List<Future<Long>> listaFutures = new ArrayList<>();

        long intervalo = N / NTAREFAS;

        // cria NTAREFAS tarefas cada uma com um pedaço do intervalo
        for (int i = 0; i < NTAREFAS; i++) {
            long inicio = i * intervalo + 1;
            long fim = (i == NTAREFAS - 1) ? N : (i + 1) * intervalo;

            PrimoCallable tarefa = new PrimoCallable(inicio, fim);

            Future<Long> futuro = executor.submit(tarefa);
            listaFutures.add(futuro);
        }

        // Recupera resultados
        long totalPrimos = 0;
        for (Future<Long> futuro : listaFutures) {
            try {
                totalPrimos += futuro.get();
            } catch (InterruptedException | ExecutionException e) {
                e.printStackTrace();
            }
        }

        executor.shutdown();

        System.out.println("Quantidade total de números primos entre 1 e " + N + ": " + totalPrimos);
    }
}
