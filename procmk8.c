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

  int shmidMutex;
  int chaveMutex = 6;
  if ((shmidMutex = shmget(chaveMutex, (sizeof(sem_t *)), IPC_CREAT | 0600)) <
      0) {
    printf("\n - Erro na criação da memória compartilhada (Mutex) - \n");
  }
  sem_t *mutex = (sem_t *)shmat(shmidMutex, NULL, 0);
  if (mutex < 0) {
    printf("\n - Erro na alocação (Mutex) - \n");
  }
  if ((mutex = sem_open("mutex", O_CREAT, 0644, 1)) == SEM_FAILED) {
    printf("\n Erro na criação do Mutex\n");
    exit(1);
  }

  int shmidTurnstile1;
  int chaveTurnstile1 = 7;
  if ((shmidTurnstile1 =
           shmget(chaveTurnstile1, (sizeof(sem_t *)), IPC_CREAT | 0600)) < 0) {
    printf("\n - Erro na criação da memória compartilhada (Turnstile1) - \n");
  }
  sem_t *turnstile1 = (sem_t *)shmat(shmidTurnstile1, NULL, 0);
  if (turnstile1 < 0) {
    printf("\n - Erro na alocação (Turnstile1) - \n");
  }

  if ((turnstile1 = sem_open("turnstile1", O_CREAT, 0644, 0)) == SEM_FAILED) {
    printf("\n Erro na criação do Turnstile1\n");
    exit(1);
  }

  int shmidTurnstile2;
  int chaveTurnstile2 = 8;
  if ((shmidTurnstile2 =
           shmget(chaveTurnstile2, (sizeof(sem_t *)), IPC_CREAT | 0600)) < 0) {
    printf("\n - Erro na criação da memória compartilhada (Turnstile2) - \n");
  }
  sem_t *turnstile2 = (sem_t *)shmat(shmidTurnstile2, NULL, 0);
  if (turnstile2 < 0) {
    printf("\n - Erro na alocação (Turnstile2) - \n");
  }

  if ((turnstile2 = sem_open("turnstile2", O_CREAT, 0644, 1)) == SEM_FAILED) {
    printf("\n Erro na criação do Turnstile2\n");
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

  int loop = 0;
  do {
    double validatorBase = 0;
    for (int l = 0; l < tam; l++) {
      validatorBase += varsX[l];
    }

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

    // BarrIni ------
    sem_wait(mutex); // Bloqueia para só 1 processo aumentar o contador por vez
    (*contador)++;

    // Quando o contador for igual ao número de processos
    // ou seja, todos os processos acabaram a iteração
    if (*contador == np) {
      sem_wait(turnstile2);
      sem_post(turnstile1); // Libera 1 dos processos que estão esperando
    }
    sem_post(mutex);

    sem_wait(turnstile1);
    sem_post(turnstile1);
    // BarrIni ------

    double validator = 0;
    for (int l = 0; l < tam; l++) {
      validator += varsX[l];
    }

    // BarrFim ------
    sem_wait(mutex); // Bloqueia para só 1 processo aumentar o contador por vez
    (*contador)--;

    if (*contador == 0) {
      sem_wait(turnstile1);
      sem_post(turnstile2); // Libera 1 dos processos que estão esperando
    }
    sem_post(mutex);

    sem_wait(turnstile2);
    sem_post(turnstile2);
    // BarrFim ------

    // Realiza o cálculo de convergência
    if (fabs(validatorBase - validator) < 0.000001) {
      printf("\nproc%d validator %0.7f validatorbase %0.7f\n", id, validator,
             validatorBase);
      printf("\nproc%d Convergido em %d iterações!\n", id, loop);
      break;
    }

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
  sem_unlink("mutex");
  sem_destroy(mutex);
  shmdt(mutex);
  shmctl(shmidMutex, IPC_RMID, NULL);
  sem_unlink("turnstile1");
  sem_destroy(turnstile1);
  shmdt(turnstile1);
  shmctl(shmidTurnstile1, IPC_RMID, NULL);
  sem_unlink("turnstile2");
  sem_destroy(turnstile2);
  shmdt(turnstile2);
  shmctl(shmidTurnstile2, IPC_RMID, NULL);
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
