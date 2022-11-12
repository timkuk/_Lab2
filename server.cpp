#pragma warning( disable : 4996)
#include <stdio.h>
#include <string>
#include <sstream>
#include <time.h>
#include <fstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stddef.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <netdb.h>
#include <arpa/inet.h>

#define MY_PORT 8081

#define PRINTNUSERS if (nclients) printf("%d user onnline\n",nclients);else printf("No User on line\n");

void* AcceptClient(void * client_socket);
void printSquareMatrix(int * m, int size);
int* createSquareMatrix(int size);

int* m_one;
int* m_two;
int* m_result;
int count_calls = 0;
int matrix_size;
int matrix_col_now;
int nclients;
int wasclients;
int max_nclients;
int count_operations;
clock_t start;

int main(int argc, char* argv[]) {
  char buff[1024];
  printf("SERVER STARTED ON PORT %d\n", MY_PORT);
  char host[256];
  char* ip;
  char* ip_i;
  struct hostent * host_entry;
  int hostname = gethostname(host, sizeof(host));
  printf("SERVER NAME: %s\n", host);
  host_entry = gethostbyname(host);
  int iter = 0;
  while ((struct in_addr*) host_entry -> h_addr_list[iter]) {
    ip = inet_ntoa(*((struct in_addr * ) host_entry -> h_addr_list[iter++]));
    printf("Server address: %s : %d\n", ip, MY_PORT);
  }

  int sock;

  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket");
    exit(1);
  }

  sockaddr_in local_addr;
  local_addr.sin_family = AF_INET;
  local_addr.sin_port = htons(MY_PORT);
  local_addr.sin_addr.s_addr = 0;

  if (bind(sock, (sockaddr * ) & local_addr, sizeof(local_addr))) {
    close(sock);
    perror("bind");
    exit(2);
  }
  
  if (listen(sock, 0x100)) {
    printf("Error listen\n");
    close(sock);
    return 1;
  }
  printf("Enter count clients: ");
  scanf("%d", & max_nclients);
  printf("Enter matrix size: ");
  scanf("%d", & matrix_size);
  
  if (matrix_size % max_nclients > 0) {
    count_operations = matrix_size / max_nclients + 1;
  } else {
    count_operations = matrix_size / max_nclients;
  }
  printf("Operations per client = %d\n", count_operations);
  
  m_one = createSquareMatrix(matrix_size);
  m_two = createSquareMatrix(matrix_size);
  m_result = new int[matrix_size];
  
  printf("Matrix A:\n");
  printSquareMatrix(m_one, matrix_size);
  printf("\nMatrix B:\n");
  printSquareMatrix(m_two, matrix_size);
  
  printf("Waiting connections...\n\n");
  
  int client_socket;
  sockaddr_in client_addr;
  socklen_t client_addr_size = sizeof(client_addr);
  while ((client_socket = accept(sock, (sockaddr*) & client_addr, & client_addr_size))) {
    if (wasclients >= max_nclients) {
      break;
    }
    nclients++;
    wasclients++;
    printf("New client accepted!\n");
    pthread_t threadId;
    pthread_create( & threadId, NULL, & AcceptClient, & client_socket);
    printf("Wait connectings..\n");
  }
  return 0;
}

void* AcceptClient(void* client_socket) {
  int sock;
  sock = ((int* ) client_socket)[0];
  char* buff = new char[1024];
  buff[0] = (char) matrix_size;
  buff[1] = (char) count_operations;
  send(sock, buff, sizeof(int) * 2, 0);
  count_calls++;
  for (int ii = 0; ii < count_operations; ii++) {
    if(count_calls == max_nclients) {
       matrix_col_now -= max_nclients + 1;
       count_operations--;
    }
    if (matrix_col_now >= matrix_size) {
      break;
    }
    
    int current_col = matrix_col_now;
    if(count_calls == max_nclients) { matrix_col_now += max_nclients + 1; }
    else { matrix_col_now += max_nclients; }
    int i, j;
    for (i = 0; i < matrix_size * matrix_size; i++) {
      buff[i] = static_cast<char>(m_two[i]);
    }
    for (i, j = current_col; i < (matrix_size * matrix_size) + matrix_size; i++, j+=matrix_size) {
      buff[i] = static_cast<char>(m_one[j]);
    }
    send(sock, buff, sizeof(int) * (matrix_size * matrix_size + matrix_size), 0);
    
    recv(sock, buff, sizeof(int) * matrix_size, 0);
    printf("Result column: ");
    for (i = 0, j = current_col; i < matrix_size; i++, j+=matrix_size) {
      m_result[j] = static_cast<int>(buff[i]);
      printf("%d ", m_result[j]);
    }
    printf("\n\n");
    printSquareMatrix(m_result, matrix_size);
    printf("\n");
  }
  nclients--;
  printf("disconnect\n\n");
  close(sock);
  return 0;
}

int* createSquareMatrix(int size) {
  int* arr = new int[size * size];
  for (int i = 0; i < size * size; i++) {
    arr[i] = i;
  }
  return arr;
}

void printSquareMatrix(int * m, int size) {
  for (int i = 0; i < size * size; i++) {
    printf("%d ", m[i]);
    if ((i + 1) % size == 0) {
      printf("\n");
    }
  }
}

