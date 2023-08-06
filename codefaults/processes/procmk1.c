#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

void geraMatrizes(int tam, int matriz[tam][tam], int resultB[tam],
                  double varsX[tam], int a3) {
  for (int i = 0; i < tam; i++) {
    int sum = 0;
    for (int j = 0; j < tam; j++) {
      // Quando não está na diagonal
      if (i != j) {
        matriz[i][j] = rand() % a3; // Gera números aleatórios
        sum += abs(matriz[i][j]);
      }
    }
    matriz[i][i] =
        sum +
        rand() % a3; // Gera um número maior para fazer a diagonal dominante
  }

  for (int i = 0; i < tam; i++) {
    resultB[i] = rand() % a3;
  }

  // Inicializa array com as variáveis X sendo 0
  for (int i = 0; i < tam; i++) {
    varsX[i] = 0;
  }
}
// --------------------------- // --------------------------- //
void jacobi(int tam, int matrizA[tam][tam], int resultB[tam],
            double varsX[tam]) {
  int loop = 1;
  int tes = 0;
  do {
    double validatorBase = 0;
    for (int l = 0; l < tam; l++) {
      validatorBase += varsX[l];
    }
    // -------------

    for (int j = 0; j < tam; j++) {
      double o = 0;
      for (int t = 0; t < tam; t++) {
        if (t != j) {
          o = o + matrizA[j][t] * varsX[t];
        }
      }

      // Atribui ao X o resultado de B menos a soma, dividido pelo A dominante
      varsX[j] = (resultB[j] - o) / matrizA[j][j];
    }

    // -------------
    double validator = 0;
    for (int l = 0; l < tam; l++) {
      validator += varsX[l];
    }

    if (tes % 10 == 0) {
      printf("tes: %f\n", validatorBase);
      printf("te: %f\n", validator);
    }
    tes++;

    if ((fabs(validatorBase - validator) < 0.0000000001) || tes > 100) {
      loop = 0;
    }

  } while (loop);
}
// --------------------------- // --------------------------- //
void printaTudo(int tam, int matrizA[tam][tam], int resultB[tam],
                double varsX[tam]) {

  printf("Tamanho dos vetores: %d\n\n", tam);
  printf("Matriz A: \n\n");
  for (int i = 0; i < tam; i++) {
    for (int j = 0; j < tam; j++) {
      printf("%d\t", matrizA[i][j]);
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
  int a2 = 10;
  int a3 = 1000;
  int np = 1;
  int tam;

  if (argc == 3) {
    np = atoi(argv[1]);
    tam = atoi(argv[2]);
    printf("Programa executando!\n");
    printf(" 1- Tamanho dos vetores: %d\n", tam);
    printf(" 2- Coeficientes de 0 - 999.999\n");
    printf(" %d Processos\n", np);
  } else {
    printf("Opções Inválidas! \n");
    return 0;
  }

  // Seeda o gerador de números aleatórios com o tempo atual
  // (assim os números são aleatórios em cada execução)
  srand(time(NULL));

  // O tamanho é um número aleatório (min = 2)
  if (tam == 1) {
    tam++;
  }

  int matrizA[tam][tam];
  int resultB[tam];
  double varsX[tam];

  geraMatrizes(tam, matrizA, resultB, varsX, a3);

  jacobi(tam, matrizA, resultB, varsX);

  if (tam <= 20) {
    printaTudo(tam, matrizA, resultB, varsX);
  }
}
