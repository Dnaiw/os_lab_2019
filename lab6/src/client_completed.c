#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <errno.h>
#include <getopt.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "lib/modulo_factorial.h"

struct Server {
  char ip[255];
  int port;
};

struct ServerTask {
  struct Server server;
  uint64_t begin;
  uint64_t end;
  uint64_t mod;
  uint64_t result;
};

bool ConvertStringToUI64(const char *str, uint64_t *val) {
  char *end = NULL;
  unsigned long long i = strtoull(str, &end, 10);
  if (errno == ERANGE) {
    fprintf(stderr, "Out of uint64_t range: %s\n", str);
    return false;
  }

  if (errno != 0)
    return false;

  *val = i;
  return true;
}

void *ServerCompute(void *args) {
  struct ServerTask *task = (struct ServerTask *)args;

  struct hostent *hostname = gethostbyname(task->server.ip);
  if (hostname == NULL) {
    fprintf(stderr, "gethostbyname failed with %s\n", task->server.ip);
    pthread_exit(NULL);
  }

  struct sockaddr_in server;
  server.sin_family = AF_INET;
  server.sin_port = htons(task->server.port);
  server.sin_addr.s_addr = *((unsigned long *)hostname->h_addr);

  int sck = socket(AF_INET, SOCK_STREAM, 0);
  if (sck < 0) {
    fprintf(stderr, "Socket creation failed!\n");
    pthread_exit(NULL);
  }

  if (connect(sck, (struct sockaddr *)&server, sizeof(server)) < 0) {
    fprintf(stderr, "Connection failed to %s:%d\n", task->server.ip, task->server.port);
    close(sck);
    pthread_exit(NULL);
  }

  // Отправляем данные серверу
  char task_data[sizeof(uint64_t) * 3];
  memcpy(task_data, &task->begin, sizeof(uint64_t));
  memcpy(task_data + sizeof(uint64_t), &task->end, sizeof(uint64_t));
  memcpy(task_data + 2 * sizeof(uint64_t), &task->mod, sizeof(uint64_t));

  if (send(sck, task_data, sizeof(task_data), 0) < 0) {
    fprintf(stderr, "Send failed\n");
    close(sck);
    pthread_exit(NULL);
  }

  // Получаем результат от сервера
  char response[sizeof(uint64_t)];
  if (recv(sck, response, sizeof(response), 0) < 0) {
    fprintf(stderr, "Receive failed\n");
    close(sck);
    pthread_exit(NULL);
  }

  memcpy(&task->result, response, sizeof(uint64_t));
  close(sck);
  pthread_exit(NULL);
}

int main(int argc, char **argv) {
  uint64_t k = -1;
  uint64_t mod = -1;
  char servers[255] = {'\0'};

  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"k", required_argument, 0, 0},
                                      {"mod", required_argument, 0, 0},
                                      {"servers", required_argument, 0, 0},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "", options, &option_index);

    if (c == -1)
      break;

    switch (c) {
    case 0: {
      switch (option_index) {
      case 0:
        ConvertStringToUI64(optarg, &k);
        break;
      case 1:
        ConvertStringToUI64(optarg, &mod);
        break;
      case 2:
        memcpy(servers, optarg, strlen(optarg));
        break;
      default:
        printf("Index %d is out of options\n", option_index);
      }
    } break;

    case '?':
      printf("Arguments error\n");
      break;
    default:
      fprintf(stderr, "getopt returned character code 0%o?\n", c);
    }
  }

  if (k == -1 || mod == -1 || !strlen(servers)) {
    fprintf(stderr, "Usage: %s --k <value> --mod <value> --servers <path>\n", argv[0]);
    return 1;
  }

  FILE *servers_file = fopen(servers, "r");
  if (!servers_file) {
    fprintf(stderr, "Failed to open servers file: %s\n", servers);
    return 1;
  }

  unsigned int servers_num = 0;
  struct Server *to = malloc(sizeof(struct Server) * 255); // Максимум 255 серверов
  while (fscanf(servers_file, "%254[^:]:%d\n", to[servers_num].ip, &to[servers_num].port) == 2) {
    servers_num++;
  }
  fclose(servers_file);

  if (servers_num == 0) {
    fprintf(stderr, "No servers found in the file.\n");
    free(to);
    return 1;
  }

  struct ServerTask tasks[servers_num];
  pthread_t threads[servers_num];
  uint64_t chunk_size = k / servers_num;
  uint64_t begin = 1;

  for (unsigned int i = 0; i < servers_num; i++) {
    tasks[i].server = to[i];
    tasks[i].begin = begin;
    tasks[i].end = (i == servers_num - 1) ? k : begin + chunk_size - 1;
    tasks[i].mod = mod;
    begin = tasks[i].end + 1;

    pthread_create(&threads[i], NULL, ServerCompute, (void *)&tasks[i]);
  }

  uint64_t result = 1;
  for (unsigned int i = 0; i < servers_num; i++) {
    pthread_join(threads[i], NULL);
    result = MultModulo(result, tasks[i].result, mod);
  }

  printf("Final result: %llu\n", result);

  free(to);
  return 0;
}
