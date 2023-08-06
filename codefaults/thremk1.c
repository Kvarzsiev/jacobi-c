#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int a3 = 1000000;
// --------------------------- // --------------------------- //
struct gera_vetores_params {
  int id;
  int nth;
  int tam;
  int a3;
  int *matriz;
  int *resultB;
  double *varsX;
};
typedef struct gera_vetores_params VETORES;
// --------------------------- // --------------------------- //
void *jacobi(int nth, int id, int tam, int *matrizA, int *resultB,
             double *varsX) {

  int it = 0;
  do {
    for (int j = id; j < tam; j += nth) {
      double o = 0;

      for (int t = 0; t < tam; t++) {
        if (t != j) {
          o = o + matrizA[(j * tam) + t] * varsX[t];
        }
      }

      // Atribui ao X o resultado de B menos a soma, dividido pelo A dominante
      varsX[j] = (resultB[j] - o) / matrizA[(j * tam) + j];
    }

    it++;
  } while (it < 1000);

  return NULL;
}
// --------------------------- // --------------------------- //
void *geraMatrizes(void *params) {
  VETORES *p = (VETORES *)params;
  int id = p->id;
  int nth = p->nth;
  int tam = p->tam;
  int a3 = p->a3;
  int *matriz = p->matriz;
  int *resultB = p->resultB;
  double *varsX = p->varsX;

  for (int i = id; i < tam; i += nth) {
    int sum = 0;
    for (int j = 0; j < tam; j++) {
      // Quando não está na diagonal
      if (i != j) {
        matriz[(i * tam) + j] = rand() % a3; // Gera números aleatórios
        sum += abs(matriz[(i * tam) + j]);
      }
    }
    matriz[(i * tam) + i] =
        sum +
        rand() % a3; // Gera um número maior para fazer a diagonal dominante
  }

  for (int i = id; i < tam; i += nth) {
    resultB[i] = rand() % a3;
  }

  // Inicializa array com as variáveis X sendo 0
  for (int i = id; i < tam; i += nth) {
    varsX[i] = 0;
  }

  jacobi(nth, id, tam, matriz, resultB, varsX);

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

  printf("\n// --------------------------- // --------------------------- "
         "//\n\n");
  printf("Vetor B: \n\n");
  for (int b = 0; b < tam; b++) {
    printf("%d\t", resultB[b]);
  }
  printf("\n// --------------------------- // --------------------------- "
         "//\n\n");

  printf("Vetor X: \n\n");
  for (int x = 0; x < tam; x++) {
    printf("%f\t", varsX[x]);
  }
  printf("\n// --------------------------- // --------------------------- "
         "//\n\n");
}
// --------------------------- // --------------------------- //
int main(int argc, char *argv[]) {
  int nth, tam;
  int id = 0;
  pthread_t *tid = NULL;
  VETORES *vetores;

  if (argc == 3) {
    nth = atoi(argv[1]);
    tam = atoi(argv[2]);
    printf("Programa executando!\n");
    printf(" 1- Tamanho dos vetores: %d\n", tam);
    printf(" 2- Coeficientes de 0 - 999.999\n");
    printf(" %d Processos\n", nth);
  } else {
    printf("Opções Inválidas! \n");
    return 0;
  }

  // Seeda o gerador de números aleatórios com o tempo atual
  // (assim os números são aleatórios em cada execução)
  srand(time(NULL));

  tid = (pthread_t *)malloc(nth * sizeof(pthread_t));
  vetores = (VETORES *)malloc((nth + 1) * sizeof(VETORES));
  int *matriz = malloc(tam * tam * sizeof(int));
  int *resultB = malloc(tam * sizeof(int));
  double *varsX = malloc(tam * sizeof(double));

  if (matriz == NULL || resultB == NULL || varsX == NULL || vetores == NULL ||
      tid == NULL) {
    printf("\nErro de alocação de memória.\n");

    free(tid);
    free(vetores);
    free(matriz);
    free(resultB);
    free(varsX);

    return 0;
  }

  for (int i = 0; i < nth; i++) {
    vetores[i].id = i;
    vetores[i].nth = nth;
    vetores[i].tam = tam;
    vetores[i].a3 = a3;
    vetores[i].matriz = matriz;
    vetores[i].resultB = resultB;
    vetores[i].varsX = varsX;

    pthread_create(&tid[i], NULL, geraMatrizes, (void *)&vetores[i]);
  }

  for (int j = 0; j < nth; j++) {
    pthread_join(tid[j], NULL);
  }

  if (tam <= 20)
    printaTudo(tam, matriz, resultB, varsX);

  // Limpando
  free(tid);
  free(vetores);
  free(matriz);
  free(resultB);
  free(varsX);

  return 1;
}
