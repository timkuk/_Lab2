#pragma warning(disable: 4996)
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <omp.h>
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <sys/types.h>
#include <stddef.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8081

char SERVERADDR[15] = "127.0.1.1";

void printSquareMatrix(int* m, int size);
void printSquareMatrixCol(int* m, int size);
int* calculateMatrixCol(int* m, int* m_row, int size);

int main(int argc, char* argv[]) {
  printf("SERVER IP: %s\n", SERVERADDR);
  char buff[1024];

  printf("CLIENT STARTED\n");

  int sock;
  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    printf("Socket error\n");
    return 1;
  }

  struct sockaddr_in dest_addr;
  dest_addr.sin_family = AF_INET;
  dest_addr.sin_port = htons(PORT);
  dest_addr.sin_addr.s_addr = inet_addr(SERVERADDR);

  int iResult = connect(sock, (struct sockaddr*) & dest_addr, sizeof(dest_addr));
  if (iResult < 0) {
    close(sock);
    printf("Unable to connect to the server\n");
    return 1;
  }

  int nsize;
  int matrixSize;
  int count_operations;
  nsize = recv(sock, buff, sizeof(int) * 2, 0);

  matrixSize = (int)buff[0];
  count_operations = (int)buff[1];

  printf("matrix size %d \n", matrixSize);

  int* matrixA = new int[matrixSize * matrixSize];
  int* matrixBCol = new int[matrixSize];
  int current_op_count = 0;
  while (current_op_count < count_operations &&
    (nsize = recv(sock, buff, sizeof(int) * (matrixSize * matrixSize + matrixSize), 0)) > 0) {
    current_op_count++;

    int i, j;
    for (i = 0, j = 0; i < (matrixSize * matrixSize) + matrixSize; i++) {
      if (i < matrixSize * matrixSize) {
        matrixA[i] = static_cast<int>(buff[i]);
      } else {
        matrixBCol[j] = static_cast<int>(buff[i]);
        j++;
      }
    }

    printf("\nMatrix: \n");
    printSquareMatrix(matrixA, matrixSize);
    printf("\nMatritx column:  \n");
    printSquareMatrixCol(matrixBCol, matrixSize);
    
    int* result = calculateMatrixCol(matrixA, matrixBCol, matrixSize);
    printf("\nResult matrix column: \n");
    printSquareMatrixCol(result, matrixSize);
    printf("\n");

    for (int i = 0; i < matrixSize; i++) {
      buff[i] = static_cast<char>(result[i]);
    }
    send(sock, buff, sizeof(int) * matrixSize, 0);
  }

  close(sock);
  delete[] matrixA;
  delete[] matrixBCol;
  return 0;
}

void printSquareMatrix(int * m, int size) {
  for (int i = 0; i < size * size; i++) {
    printf("%d ", *(m + i));
    if ((i + 1) % size == 0) {
      printf("\n");
    }
  }
}

void printSquareMatrixCol(int * m, int size) {
  for (int i = 0; i < size; i++) {
    printf("%d ", *(m + i));
  }
}

int* calculateMatrixCol(int* matrix, int* column, int size) {
  int* res = new int[size];
  for (int i = 0; i < size; i++) {
    res[i] = 0;
  }
  for (int i = 0; i < size; i++) {
      for (int j = 0; j < size; j++) {
          res[i] += matrix[i * size + j] * column[j];
      }
  }
  return res;
}

