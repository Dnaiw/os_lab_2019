/********************************************************
 * Пример использования POSIX потоков с мьютексом.
 * Использует многопоточность для вычисления факториала по модулю.
 ********************************************************/
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>

// Определяем структуру для данных
struct FactorialData {
  int k;
  int mod;
  int mod_value;
  int current_num;
  int last_multiplier;
};

pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;

// Прототипы функций
void *factorial(void *args);
void do_wrap_up(int result);

int main(int argc, char **argv) {
    int k = -1;
    int pnum = -1;
    int mod = -1;

    // Обработка аргументов командной строки
    static struct option long_options[] = {
        {"pnum", required_argument, 0, 'p'},
        {"mod", required_argument, 0, 'm'},
        {0, 0, 0, 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "k:", long_options, NULL)) != -1) {
        switch (opt) {
            case 'k':
                k = atoi(optarg);
                break;
            case 'p':
                pnum = atoi(optarg);
                break;
            case 'm':
                mod = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Usage: %s -k <value> --pnum=<value> --mod=<value>\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (k == -1 || pnum == -1 || mod == -1) {
        fprintf(stderr, "Missing required arguments.\n");
        fprintf(stderr, "Usage: %s -k <value> --pnum=<value> --mod=<value>\n", argv[0]);
        return 1;
    }

    struct FactorialData current;
    current.k = k;
    current.mod = mod;
    current.mod_value = k % mod;
    current.current_num = current.mod_value == 0 ? mod : current.mod_value;
    current.last_multiplier = current.current_num;

    pthread_t threads[pnum];

    // Создаем потоки
    for (int i = 0; i < pnum; i++) {
        if (pthread_create(&threads[i], NULL, factorial, (void *)&current) != 0) {
            perror("pthread_create");
            exit(1);
        }
    }

    // Ожидаем завершения потоков
    for (int i = 0; i < pnum; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("pthread_join");
            exit(1);
        }
    }

    do_wrap_up(current.current_num);

    return 0;
}

// Функция, выполняемая в потоке
void *factorial(void *args) {
    struct FactorialData *current = (struct FactorialData *)args;
    int multiplier;
    int work;

    while (1) {
        pthread_mutex_lock(&mut);
        multiplier = current->last_multiplier + current->mod;

        if (multiplier > current->k) {
            pthread_mutex_unlock(&mut);
            break;
        }

        work = current->current_num * multiplier;
        current->current_num = work;
        current->last_multiplier = multiplier;
        pthread_mutex_unlock(&mut);
    }

    return NULL;
}

// Вывод результата
void do_wrap_up(int result) {
    printf("All done, result = %d\n", result);
}
