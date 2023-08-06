#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

void geraMatrizes(int tam, int *matriz, int *resultB, double *varsX, int a3,
                  int ini, int np) {
  for (int i = ini; i < tam; i += np) {
    int sum = 0;
    for (int j = 0; j < tam; j++) {
      // Quando não está na diagonal
      if (i != j) {
        matriz[(i * tam) + j] = 1; // Gera números aleatórios
        sum += abs(matriz[(i * tam) + j]);
      }
    }
    matriz[(i * tam) + i] =
        sum + 1; // Gera um número maior para fazer a diagonal dominante
  }

  for (int i = ini; i < tam; i += np) {
    resultB[i] = rand() % a3;
  }

  // Inicializa array com as variáveis X sendo 0
  for (int i = ini; i < tam; i += np) {
    varsX[i] = 0;
  }
}
// --------------------------- // --------------------------- //
void jacobi(int tam, int *matrizA, int *resultB, double *varsX, int ini,
            int np) {

  int loop = 0;
  do {
    double validatorBase = 0;
    for (int l = 0; l < tam; l++) {
      validatorBase += varsX[l];
    }
    // -------------

    for (int j = ini; j < tam; j += np) {
      double o = 0;
      for (int t = 0; t < tam; t++) {
        if (t != j) {
          o = o + matrizA[(j * tam) + t] * varsX[t];
        }
      }

      // Atribui ao X o resultado de B menos a soma, dividido pelo A dominante
      varsX[j] = (resultB[j] - o) / matrizA[(j * tam) + j];
    }

    // -------------
    double validator = 0;
    for (int l = 0; l < tam; l++) {
      validator += varsX[l];
    }

    if (fabs(validatorBase - validator) < 0.000001) {
      printf("\nloopstop\n");
      loop = 101;
    }

    if (loop % 17 == 0) {
      // printf("\ntttt: %d\n", loop);
      printf("\ntes: %f\n", fabs(validatorBase - validator));
      // printf("\nte: %f\n", validator);
    }

    loop++;
  } while (loop < 100);
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
  int a3 = 1000000;
  int np, tam, shmidA, shmidB, shmidX;

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

  int chaveA = 1;
  // Alocando a matrizA
  if ((shmidA = shmget(chaveA, (tam * tam * sizeof(int)), IPC_CREAT | 0600)) <
      0) {
    printf("\n - Erro na criação da memória compartilhada (MatrizA) - \n");
  }
  // int(*matrizA)[tam][tam] = (int(*)[tam][tam])shmat(shmidA, NULL, 0);
  int *matrizA = (int(*))shmat(shmidA, NULL, 0);
  // int matrizA[tam][tam];
  if (*matrizA < 0) {
    printf("\n - Erro na alocação (MatrizA) - \n");
  }

  int chaveB = 2;
  // Alocando o vetorB
  if ((shmidB = shmget(chaveB, (tam * sizeof(int)), IPC_CREAT | 0600)) < 0) {
    printf("\n - Erro na criação da memória compartilhada (Vetor B) - \n");
  }
  // int(*resultB)[tam] = (int(*)[tam])shmat(shmidB, NULL, 0);
  int *resultB = (int(*))shmat(shmidB, NULL, 0);
  if (*resultB < 0) {
    printf("\n - Erro na alocação (Vetor B) - \n");
  }

  int chaveX = 3;
  // Alocando o vetorB
  if ((shmidX = shmget(chaveX, (tam * sizeof(double)), IPC_CREAT | 0600)) < 0) {
    printf("\n - Erro na criação da memória compartilhada (Vetor X) - \n");
  }
  // double(*varsX)[tam] = (double(*)[tam])shmat(shmidX, NULL, 0);
  double *varsX = (double(*))shmat(shmidX, NULL, 0);
  // double varsX[tam];
  if (*varsX < 0) {
    printf("\n - Erro na alocação (Vetor X) - \n");
  }

  int pid;
  int id = 0;
  for (int i = 1; i < np; i++) {
    pid = fork();
    if (pid == 0) {
      id = i;
      break;
    }
  }

  geraMatrizes(tam, matrizA, resultB, varsX, a3, id, np);

  jacobi(tam, matrizA, resultB, varsX, id, np);

  // Pai
  if (pid > 0 || np == 1) {
    for (int i = 0; i < np; i++) {
      wait(NULL);
    }

    if (tam <= 20) {
      printaTudo(tam, matrizA, resultB, varsX);
    }
  }
  // Filhos
  else if (pid == 0) {
    exit(0);
  }

  // Limpando
  shmdt(matrizA);
  shmdt(resultB);
  shmdt(varsX);
  shmctl(shmidA, IPC_RMID, NULL);
  shmctl(shmidB, IPC_RMID, NULL);
  shmctl(shmidX, IPC_RMID, NULL);
  return 1;
}
