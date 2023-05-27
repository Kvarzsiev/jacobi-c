#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// TODO gerar números negativos
void geraMatrizes(int tam, int matriz[tam][tam], int resultB[tam],
                  double varsX[tam]) {
  for (int i = 0; i < tam; i++) {
    int sum = 0;
    for (int j = 0; j < tam; j++) {
      // Quando não está na diagonal
      if (i != j) {
        matriz[i][j] = rand() % 1000; // Gera números aleatórios de 0 à 999
        sum += abs(matriz[i][j]);
      }
    }
    matriz[i][i] =
        sum +
        rand() % 1000; // Gera um número maior para fazer a diagonal dominante
  }

  for (int i = 0; i < tam; i++) {
    resultB[i] = rand() % 1000;
  }

  // Inicializa array com as variáveis X sendo 0
  for (int i = 0; i < tam; i++) {
    varsX[i] = 0;
  }
}
// --------------------------- // --------------------------- //
void jacobi(int tam, int matrizA[tam][tam], int resultB[tam],
            double varsX[tam]) {
  double validator = 0;
  do {
    validator = 0;
    for (int j = 0; j < tam; j++) {
      double o = 0;
      for (int t = 0; t < tam; t++) {
        if (t != j) {
          o = o + matrizA[j][t] * varsX[t];
        }
      }

      // Declara um placeholder para manter o valor de x nessa iteração
      double ph = (1.0 / matrizA[j][j]) * (resultB[j] - o);
      // Soma ao validator o valor de x da última iteração dividido pelo valor
      // dessa, de modo que se os valores forem iguais nas duas, somará 1
      validator += (varsX[j] / ph);
      // atribui o valor do placeholder ao x
      varsX[j] = ph;
    }

    // O loop se repete enquanto o validator for diferente do tamanho,
    // pois quando uma iteração repetir os mesmos valores da anterior,
    // a soma total (de 1 em 1) dará exatamente o tamanho das matrizes
    // significando que o x já foi aproximado ao máximo
  } while (validator != tam);
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
int main() {
  // Seeda o gerador de números aleatórios com o tempo atual
  // (assim os números são aleatórios em cada execução)
  srand(time(NULL));

  // O tamanho é um número aleatório de 2 até 10
  int tam = (rand() % 10) + 1;
  if (tam == 1) {
    tam++;
  }
  int matrizA[tam][tam];
  int resultB[tam];
  double varsX[tam];

  geraMatrizes(tam, matrizA, resultB, varsX);

  jacobi(tam, matrizA, resultB, varsX);

  printaTudo(tam, matrizA, resultB, varsX);
}
