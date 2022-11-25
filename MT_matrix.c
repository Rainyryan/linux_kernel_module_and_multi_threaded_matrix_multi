#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <stdbool.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

int rows_total, cols_total, rows_total_2;
int NUM_THREADS; // Number of threads to spawn
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
// Structure to pass to threads
struct Matrices
{
    int id;
    int **a;
    int **b;
    int **result;
};

// Thread function
void *runner(void *args)
{
    struct Matrices *thread_data = (struct Matrices *)args;
    int id = (*thread_data).id;
    int sum = 0;

    int rows_local = rows_total / NUM_THREADS; // number of rows each thread has to do
    if (id == NUM_THREADS - 1)                 // last thread does the remaining rows
        rows_local += rows_total % NUM_THREADS;

    for (int i = 0; i < rows_local; i++)
    {
        for (int j = 0; j < cols_total; j++)
        {
            for (int k = 0; k < rows_total_2; k++)
            {
                sum += (*thread_data).a[id * (rows_total / NUM_THREADS) + i][k] * (*thread_data).b[k][j];
            }
            (*thread_data).result[id * (rows_total / NUM_THREADS) + i][j] = sum;
            sum = 0;
        }
    }

    int x = gettid();
    pthread_mutex_lock(&mutex); // prevent race condition by locking when writing
    FILE *tid = fopen("/proc/thread_info", "r+");
    fprintf(tid, "%d", x);
    fclose(tid);
    pthread_mutex_unlock(&mutex);
}

// Multithreaded multiplication algorithm
void multiply_multi(int **a, int **b, int **result)
{
    pthread_t *threads;
    threads = malloc(NUM_THREADS * sizeof(pthread_t));
    struct Matrices thread_data[NUM_THREADS];
    int i;

    for (i = 0; i < NUM_THREADS; i++)
    {
        thread_data[i].id = i;
        thread_data[i].a = a;
        thread_data[i].b = b;
        thread_data[i].result = result;
        pthread_create(&threads[i], NULL, runner, &thread_data[i]);
    }

    for (i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }
}

int main(int argc, char *argv[])
{
    srand(time(NULL));
    // read m1.txt
    NUM_THREADS = atoi(argv[1]);
    FILE *file_1 = fopen(argv[2], "r");
    int row1, column1;
    fscanf(file_1, "%d %d", &row1, &column1);
    int **matrix1 = malloc(row1 * sizeof(int *));
    for (int i = 0; i < row1; i++)
    {
        matrix1[i] = malloc(column1 * sizeof(int));
        for (int j = 0; j < column1; j++)
        {
            int n;
            fscanf(file_1, "%d", &n);
            matrix1[i][j] = n;
        }
    }
    // read m2.txt
    FILE *file_2 = fopen(argv[3], "r");
    int row2, column2;
    fscanf(file_2, "%d %d", &row2, &column2);
    int **matrix2 = malloc(row2 * sizeof(int *));
    for (int i = 0; i < row2; i++)
    {
        matrix2[i] = malloc(column2 * sizeof(int));
        for (int j = 0; j < column2; j++)
        {
            int n;
            fscanf(file_2, "%d", &n);
            matrix2[i][j] = n;
        }
    }

    rows_total = row1;
    cols_total = column2;
    rows_total_2 = row2;

    int **result_matrix = malloc(row1 * sizeof(int *));
    for (int i = 0; i < row1; i++)
        result_matrix[i] = malloc(column2 * sizeof(int));


    printf("PID:%d\n", (int)getpid());
    // set start time
    struct timespec start, end;
    double elapsed;
    clock_gettime(CLOCK_MONOTONIC, &start);
    // do multi-threaded multiplication
    multiply_multi(matrix1, matrix2, result_matrix);
    // output to result.txt
    FILE *result_file = fopen("result.txt", "w+");
    fprintf(result_file, "%d %d \n", rows_total, cols_total);
    for (int i = 0; i < rows_total; i++)
    {
        for (int j = 0; j < cols_total; j++)
        {
            int n = result_matrix[i][j];
            fprintf(result_file, "%d ", n);
        }
        fprintf(result_file, "\n");
    }

    // get info from /proc
    FILE *info = fopen("/proc/thread_info", "r+");
    char temp[10000];
    while (fgets(temp, 10000, info) != NULL)
    {
        printf("%s", temp);
    }
    fclose(info);
    // get elapsed time
    clock_gettime(CLOCK_MONOTONIC, &end);
    elapsed = (end.tv_sec - start.tv_sec);
    elapsed += (end.tv_nsec - start.tv_nsec) / 1000000000.0;
    printf("total execution time:%lf\n", elapsed);
    return 0;
}