#include <fcntl.h>
#include <math.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>

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
void jacobi(int tam, int *matrizA, int *resultB, double *varsX, int np) {
  int shmidContador;
  int chaveContador = 4;
  if ((shmidContador =
           shmget(chaveContador, (np * sizeof(int)), IPC_CREAT | 0600)) < 0) {
    printf("\n - Erro na criação da memória compartilhada (Contador) - \n");
  }
  int *contador = (int(*))shmat(shmidContador, NULL, 0);
  if (*contador < 0) {
    printf("\n - Erro na alocação (Contador) - \n");
  }

  int shmidValidatorBase;
  int chaveValidatorBase = 5;
  if ((shmidValidatorBase = shmget(chaveValidatorBase, (np * sizeof(double)),
                                   IPC_CREAT | 0600)) < 0) {
    printf(
        "\n - Erro na criação da memória compartilhada (Validator Base) - \n");
  }
  double *validatorBase = (double(*))shmat(shmidValidatorBase, NULL, 0);
  if (*validatorBase < 0) {
    printf("\n - Erro na alocação (Validator Base) - \n");
  }

  int shmidSem;
  int chaveSem = 6;
  if ((shmidSem = shmget(chaveSem, (sizeof(sem_t *)), IPC_CREAT | 0600)) < 0) {
    printf("\n - Erro na criação da memória compartilhada (Semáforo) - \n");
  }
  sem_t *sem = (sem_t *)shmat(shmidSem, NULL, 0);
  if (sem < 0) {
    printf("\n - Erro na alocação (Semáforo) - \n");
  }

  if ((sem = sem_open("sem", O_CREAT, 0644, 1)) == SEM_FAILED) {
    printf("\n Erro na criação do semáforo \n");
    exit(1);
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

  int teste;
  int loop = 0;
  do {
    for (int j = id; j < tam; j += np) {
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

    // A cada iteração do jacobi aumenta em 1 o contador
    (*contador)++;

    // Caso o contador seja igual ao número de processos
    // (ou seja, todos os processos realizaram 1 iteração)
    if (*contador == np) {
      sem_getvalue(sem, &teste);
      printf("\nsemvalue proc%d: %d\n", id, teste);
      printf("\nsempost proc%d\n", id);
      // libera o semáforo e reseta o contador
      sem_post(sem);
      *contador = 0;
    }
    // Caso contrário, espera o semáforo
    else {
      sem_getvalue(sem, &teste);
      printf("\nsemvalue proc%d: %d\n", id, teste);

      printf("\nsemWait proc%d\n", id);
      sem_wait(sem);
    }
    // printf("\nsem proc%d: %d\n", id, sem);

    printf("proc%d Vetor X: \n\n", id);
    for (int x = 0; x < tam; x++) {
      printf("%f\n", varsX[x]);
    }

    // if (id == 0)
    // printf("\nvalidatorbase1: %f\n", *validatorBase);
    // Realiza o cálculo de convergência
    double validator = 0;
    for (int l = 0; l < tam; l++) {
      validator += varsX[l];
    }

    printf("\nvalidatorproc%d: %f\n", id, validator);
    printf("\nvalidatorbase1proc%d: %f\n", id, validatorBase[id]);

    if (fabs(validatorBase[id] - validator) < 0.0000001) {
      printf("\nconvergenciaproc%d: %d\n", id,
             fabs(validatorBase[id] - validator) < 0.0000001);
      printf("\nproc%d Convergido em %d iterações!\n", id, loop);
      break;
    }

    validatorBase[id] = validator;
    printf("\nvalidatorbase2proc%d: %f\n", id, validatorBase[id]);

    loop++;
  } while (loop < 100);

  // Pai
  if (pid > 0 || np == 1) {
    for (int i = 0; i < np; i++) {
      wait(NULL);
    }

  }
  // Filhos
  else if (pid == 0) {
    exit(0);
  }

  // limpa contador e validator base
  shmdt(contador);
  shmctl(shmidContador, IPC_RMID, NULL);
  shmdt(validatorBase);
  shmctl(shmidValidatorBase, IPC_RMID, NULL);
  sem_destroy(sem);
  sem_unlink("/sem");
  shmdt(sem);
  shmctl(shmidSem, IPC_RMID, NULL);
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
  int np, tam, shmidA, shmidB, shmidX;
  int chaveA, chaveB, chaveX;

  if (argc == 3) {
    np = atoi(argv[1]);
    tam = atoi(argv[2]);
    printf("Programa executando!\n");
    printf(" 1- Tamanho dos vetores: %d\n", tam);
    printf(" %d Processos\n", np);
  } else {
    printf("Opções Inválidas!\n");
    return 0;
  }

  chaveA = 1;
  // Alocando a matrizA
  if ((shmidA = shmget(chaveA, (tam * tam * sizeof(int)), IPC_CREAT | 0600)) <
      0) {
    printf("\n - Erro na criação da memória compartilhada (MatrizA) - \n");
  }
  int *matrizA = (int(*))shmat(shmidA, NULL, 0);
  if (*matrizA < 0) {
    printf("\n - Erro na alocação (MatrizA) - \n");
  }

  chaveB = 2;
  if ((shmidB = shmget(chaveB, (tam * sizeof(int)), IPC_CREAT | 0600)) < 0) {
    printf("\n - Erro na criação da memória compartilhada (Vetor B) - \n");
  }
  int *resultB = (int(*))shmat(shmidB, NULL, 0);
  if (*resultB < 0) {
    printf("\n - Erro na alocação (Vetor B) - \n");
  }

  chaveX = 3;
  if ((shmidX = shmget(chaveX, (tam * sizeof(double)), IPC_CREAT | 0600)) < 0) {
    printf("\n - Erro na criação da memória compartilhada (Vetor X) - \n");
  }
  double *varsX = (double(*))shmat(shmidX, NULL, 0);
  if (*varsX < 0) {
    printf("\n - Erro na alocação (Vetor X) - \n");
  }

  geraMatrizes(tam, matrizA, resultB, varsX);

  jacobi(tam, matrizA, resultB, varsX, np);

  if (tam <= 20) {
    printaTudo(tam, matrizA, resultB, varsX);
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
