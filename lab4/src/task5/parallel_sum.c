#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <getopt.h>

struct SumArgs {
  int *array;
  int begin;
  int end;
};

int Sum(const struct SumArgs *args) {
  int sum = 0;
  for (int i = args->begin; i < args->end; i++) {
    sum += args->array[i];
  }
  return sum;
}

void *ThreadSum(void *args) {
  struct SumArgs *sum_args = (struct SumArgs *)args;
  int *result = malloc(sizeof(int));
  *result = Sum(sum_args);
  return (void *)result;
}

int main(int argc, char **argv) {
  uint32_t threads_num = 0;
  uint32_t array_size = 0;
  uint32_t seed = 0;

  // Чтение аргументов командной строки
  while (1) {
    static struct option options[] = {
        {"threads_num", required_argument, 0, 't'},
        {"array_size", required_argument, 0, 'a'},
        {"seed", required_argument, 0, 's'},
        {0, 0, 0, 0}};
    int option_index = 0;
    int c = getopt_long(argc, argv, "t:a:s:", options, &option_index);

    if (c == -1) break;

    switch (c) {
      case 't':
        threads_num = atoi(optarg);
        break;
      case 'a':
        array_size = atoi(optarg);
        break;
      case 's':
        seed = atoi(optarg);
        break;
      default:
        printf("Usage: %s --threads_num \"num\" --array_size \"num\" --seed \"num\"\n", argv[0]);
        return 1;
    }
  }

  if (threads_num == 0 || array_size == 0 || seed == 0) {
    printf("All arguments must be greater than 0\n");
    return 1;
  }

  // Создание массива и его заполнение
  int *array = malloc(sizeof(int) * array_size);
  srand(seed);
  for (uint32_t i = 0; i < array_size; i++) {
    array[i] = rand() % 100;
  }

  pthread_t threads[threads_num];
  struct SumArgs args[threads_num];

  // Разделение массива на части для каждого потока
  int part_size = array_size / threads_num;
  for (uint32_t i = 0; i < threads_num; i++) {
    args[i].array = array;
    args[i].begin = i * part_size;
    args[i].end = (i == threads_num - 1) ? array_size : (i + 1) * part_size;

    if (pthread_create(&threads[i], NULL, ThreadSum, (void *)&args[i])) {
      printf("Error: pthread_create failed!\n");
      free(array);
      return 1;
    }
  }

  // Сбор результатов из потоков
  int total_sum = 0;
  for (uint32_t i = 0; i < threads_num; i++) {
    int *sum;
    pthread_join(threads[i], (void **)&sum);
    total_sum += *sum;
    free(sum);
  }

  free(array);
  printf("Total sum: %d\n", total_sum);
  return 0;
}
