#include <fcntl.h>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>

sem_t mutex;
sem_t turnstile1;
sem_t turnstile2;

struct jacobi_thread {
  int id;
  int nth;
  int tam;
  int *matrizA;
  int *resultB;
  double *varsX;
  int *contador;
};
typedef struct jacobi_thread JACOBI;
// --------------------------- // --------------------------- //
void geraMatrizes(int tam, int *matriz, int *resultB, double *varsX) {
  for (int i = 0; i < tam; i++) {
    int sum = 0;
    for (int j = 0; j < tam; j++) {
      // Quando não está na diagonal
      if (i != j) {
        matriz[(i * tam) + j] = 1; // Inicializa com 1
        sum += abs(matriz[(i * tam) + j]);
      }
    }
    // Quando na diagonal, inicializa com a
    matriz[(i * tam) + i] = sum + 1; // soma total + 1, para fazer a dominante
  }

  for (int i = 0; i < tam; i++) {
    resultB[i] = i;
  }

  // Inicializa array com as variáveis X sendo 0
  for (int i = 0; i < tam; i++) {
    varsX[i] = 0;
  }
}
// --------------------------- // --------------------------- //
void *jacobi(void *params) {
  JACOBI *p = (JACOBI *)params;
  int id = p->id;
  int nth = p->nth;
  int tam = p->tam;
  int *matrizA = p->matrizA;
  int *resultB = p->resultB;
  double *varsX = p->varsX;
  int *contador = p->contador;

  int loop = 0;
  do {
    double validatorBase = 0;
    for (int l = 0; l < tam; l++) {
      validatorBase += varsX[l];
    }

    for (int j = id; j < tam; j += nth) {
      double o = 0;
      for (int t = 0; t < tam; t++) {
        if (t != j) {
          o = o + matrizA[(j * tam) + t] * varsX[t];
        }
      }

      // Atribui ao X o resultado de B menos a soma, dividido pelo A
      // dominante
      varsX[j] = (resultB[j] - o) / matrizA[(j * tam) + j];
    }

    // BarrIni ------
    sem_wait(&mutex); // Bloqueia para só 1 processo aumentar o contador por vez
    (*contador)++;

    // Quando o contador for igual ao número de processos
    // ou seja, todos os processos acabaram a iteração
    if (*contador == nth) {
      sem_wait(&turnstile2);
      sem_post(&turnstile1); // Libera 1 dos processos que estão esperando
    }
    sem_post(&mutex);

    sem_wait(&turnstile1);
    sem_post(&turnstile1);
    // BarrIni ------

    double validator = 0;
    for (int l = 0; l < tam; l++) {
      validator += varsX[l];
    }

    // BarrFim ------
    sem_wait(&mutex); // Bloqueia para só 1 processo aumentar o contador por vez
    (*contador)--;

    if (*contador == 0) {
      sem_wait(&turnstile1); // Libera 1 dos processos que estão esperando
      sem_post(&turnstile2); // Libera 1 dos processos que estão esperando
    }
    sem_post(&mutex);

    sem_wait(&turnstile2);
    sem_post(&turnstile2);
    // BarrFim ------

    // Realiza o cálculo de convergência
    if (fabs(validatorBase - validator) < 0.000001) {
      printf("\nthr%d validator %0.7f validatorbase %0.7f\n", id, validator,
             validatorBase);
      printf("\nthr%d Convergido em %d iterações!\n", id, loop);
      break;
    }

    loop++;
  } while (loop < 100);

  return NULL;
}
// --------------------------- // --------------------------- //
void printaTudo(int tam, int *matrizA, int *resultB, double *varsX) {

  printf("Tamanho dos vetores: %d\n\n", tam);
  printf("Matriz A: \n\n");
  for (int i = 0; i < tam; i++) {
    for (int j = 0; j < tam; j++) {
      printf("%d\t", matrizA[(i * tam) + j]);
    }
    printf("\n");
  }

  printf(
      "\n// --------------------------- // --------------------------- //\n\n");
  printf("Vetor B: \n\n");
  for (int b = 0; b < tam; b++) {
    printf("%d\t", resultB[b]);
  }
  printf(
      "\n// --------------------------- // --------------------------- //\n\n");

  printf("Vetor X: \n\n");
  for (int x = 0; x < tam; x++) {
    printf("%f\t", varsX[x]);
  }
  printf(
      "\n// --------------------------- // --------------------------- //\n\n");
}
// --------------------------- // --------------------------- //
int main(int argc, char *argv[]) {
  int nth, tam;
  int chaveA, chaveB, chaveX;

  if (argc == 3) {
    nth = atoi(argv[1]);
    tam = atoi(argv[2]);
    printf("Programa executando!\n");
    printf(" 1- Tamanho dos vetores: %d\n", tam);
    printf(" %d Threads\n", nth);
  } else {
    printf("Opções Inválidas!\n");
    return 0;
  }

  pthread_t *tid = (pthread_t *)malloc(nth * sizeof(pthread_t));
  JACOBI *jacobiT = (JACOBI *)malloc((nth + 1) * sizeof(JACOBI));
  int *matrizA = malloc(tam * tam * sizeof(int));
  int *resultB = malloc(tam * sizeof(int));
  double *varsX = malloc(tam * sizeof(double));
  int *contador = (int *)malloc(nth * sizeof(int));

  if (matrizA == NULL || resultB == NULL || varsX == NULL || jacobiT == NULL ||
      tid == NULL || contador == NULL) {
    printf("\nErro de alocação de memória.\n");

    free(tid);
    free(jacobiT);
    free(matrizA);
    free(resultB);
    free(varsX);
    free(contador);
    free(varsX);

    return 0;
  }

  if (sem_init(&mutex, 0, 1) < 0) {
    printf("\n - Erro na inicialização (Mutex) - \n");
  }

  if (sem_init(&turnstile1, 0, 0) < 0) {
    printf("\n - Erro na inicialização (Turnstile1) - \n");
  }

  if (sem_init(&turnstile2, 0, 1) < 0) {
    printf("\n - Erro na inicialização (Turnstile2) - \n");
  }

  geraMatrizes(tam, matrizA, resultB, varsX);

  for (int i = 0; i < nth; i++) {
    jacobiT[i].id = i;
    jacobiT[i].nth = nth;
    jacobiT[i].tam = tam;
    jacobiT[i].matrizA = matrizA;
    jacobiT[i].resultB = resultB;
    jacobiT[i].varsX = varsX;
    jacobiT[i].contador = contador;

    pthread_create(&tid[i], NULL, jacobi, (void *)&jacobiT[i]);
  }

  for (int j = 0; j < nth; j++) {
    pthread_join(tid[j], NULL);
  }

  if (tam <= 20) {
    printaTudo(tam, matrizA, resultB, varsX);
  }

  // Limpando
  free(tid);
  free(jacobiT);
  free(matrizA);
  free(resultB);
  free(varsX);
  free(contador);
  sem_destroy(&mutex);
  sem_destroy(&turnstile1);
  sem_destroy(&turnstile2);

  return 1;
}
