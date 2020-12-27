#include <stdio.h>
#include <time.h>
#include <omp.h>
#include <stdlib.h>
#include <math.h>

#define N 1000
#define c 15.0

float A[N][N]; //= {8, 1, 2, 0.5, 2, 1, 2, 0, 0, 0, 2, 0, 6, 0, 0, 0.5, 0, 0, 22, 0, 2, 0, 0, 0, 16}; //матрицы А
float b[N];    //= {17, 3, 7, 6, 12};
// результат - {180, 23, 76,140.5,226}
float result[N];
int i, j, tread_id;

//нет распараллеивания
void matrix(void)
{
    float sum = 0;
    srand((unsigned int)time(NULL));
    for (int j = 0; j < N; j++)
    {
        sum = 0; //считаем сумму по столбцам
        for (int i = 0; i < N; i++)
        {
            if (i != j)
            {
                A[i][j] = ((float)rand() / (float)(RAND_MAX)) * c - 5.0;
                sum += fabsf(A[i][j]);
            }
        }
        A[j][j] = 1.1 * (sum + fabsf(((float)rand() / (float)(RAND_MAX)) * c));
        b[j] = ((float)rand() / (float)(RAND_MAX)) * c - 5.0;
    }
}

//тестовая матрица с распараллеливанием
void new_matrix(void)
{
#pragma omp parallel for private(i,j)

    for (i = 0; i < N; i++)
    {
        for (j = 0; j < N; j++)
        {
            A[i][j] = ((float)rand() / (float)(RAND_MAX)) * c - 5.0;
        }
        b[i] = ((float)rand() / (float)(RAND_MAX)) * c - 5.0;
    }
}

void multiplication(float *x_last, float *result)
{
#pragma omp parallel for shared(A, result, b) private(j) 
//каждый поток будет иметь свою копию столбца
//параллелим по строкам матрицы А
    for (i = 0; i < N; i++)
    {
        for (j = 0; j < N; j++)
        {
            //tread_id = omp_get_thread_num();
            result[i] += A[i][j] * b[j];
            //printf("For thread %d \n", tread_id);
        }
    }
}

int main()
{
    matrix();
    omp_set_num_threads(1);
    clock_t start1 = clock();
    //new_matrix();
    multiplication(b, result);
    clock_t stop1 = clock();
    double elapsed1 = (double)(stop1 - start1) / CLOCKS_PER_SEC;
    printf("\nTime 1 thread for %d matrix =: %f\n", N, elapsed1);

    omp_set_num_threads(4);
    clock_t start3 = clock();
    //new_matrix();
    multiplication(b, result);
    clock_t stop3 = clock();
    double elapsed3 = (double)(stop3 - start3) / CLOCKS_PER_SEC;
    printf("\nTime 4 thread =: %f\n", elapsed3);

    omp_set_num_threads(8);
    clock_t start = clock();
   //new_matrix();
    multiplication(b, result);
    clock_t stop = clock();
    double elapsed = (double)(stop - start) / CLOCKS_PER_SEC;
    printf("\nTime 8 thread=: %f\n", elapsed);

    return 0;
}

//gcc -fopenmp openmp.c -o openmp && ./openmp

