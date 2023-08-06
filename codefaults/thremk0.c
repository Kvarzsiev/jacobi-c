#include <stdio.h>

int main(int argc, char *argv[]) {

  int i = 0, j = 0, iterationLimit = 30;

  // #initialize the matrix
  int a[4][4] = {
      {10, -1, 2, 0}, {-1, 11, -1, 3}, {2, -1, 10, -1}, {0, 3, -1, 8}};

  // #initialize the RHS vector
  int b[4] = {6, 25, -11, 15};

  int iteration = 0, k = 0;
  double x[4] = {0, 0, 0, 0};

  for (iteration = 0; iteration < iterationLimit; iteration++) {

    for (i = 0; i < 4; i++) {
      double o = 0;
      for (j = 0; j < 4; j++) {
        if (j != i) {
          o = o + a[i][j] * x[j];
          printf("test %f\n", o);
        }
      }
      x[i] = (1.0 / a[i][i]) * (b[i] - o);
      printf("test %f\n", x[i]);
    }
  }

  printf("%f\n", x[0]);
  printf("%f\n", x[1]);
  printf("%f\n", x[2]);
  printf("%f\n", x[3]);
}
// for (i = 0; i < 30; i++) {
//   for (j = 0; j < tam; j++) {
//     double o = 0;
//     for (t = 0; t < tam; t++) {
//       if (t != j) {
//         o = o + matrizA[j][t] * varsX[t];
//         // printf("test %f\n", o);
//       }
//     }
//     varsX[j] = (1.0 / matrizA[j][j]) * (resultB[j] - o);
//     // printf("test %f\n", varsX[j]);
//   }
// }
