#include <stdio.h>
#include <time.h>
#include <omp.h>
#include <stdlib.h>
#include <math.h>

#define N 500
#define c 15.0
#define treads_num 8

float A[N][N];//= {8, 1, 2, 0.5, 2, 1, 2, 0, 0, 0, 2, 0, 6, 0, 0, 0.5, 0, 0, 22, 0, 2, 0, 0, 0, 16}; //матрицы А
float b[N];// = {17, 3, 7, 6, 12};
// результат - {180, 23, 76,140.5,226}
float result[N];
float zeros[N] = {0};
int i, j;
float esp = 0.00001;																				  
float norma = 1;
int count=0;

float x_last[N];
float C[N];
float B[N][N];


void matrix(void)
{   
	float sum = 0;

	srand((unsigned int)time(NULL));
	for (int i = 0; i < N; i++)
	{
		sum = 0; //считаем сумму по cтрокам
		for (int j = 0; j < N; j++)
		{
			if (i != j)
			{
				A[i][j] = ((float)rand() / (float)(RAND_MAX)) * c - 5.0;
				sum += fabsf(A[i][j]);
			}
		}
		A[i][i] = 1.1 * (sum + fabsf(((float)rand() / (float)(RAND_MAX)) * c));
		b[i] = ((float)rand() / (float)(RAND_MAX)) * c - 5.0;
	}
}



void multiplication(float *x_last, float *result)
{ float k=0;
#pragma omp parallel for shared(A, result, b) private(j,k) 
//каждый поток будет иметь свою копию столбца j и переменную k
//параллелим по строкам матрицы B 
    for (i = 0; i < N; i++)
    {   k=0;
        for (j = 0; j < N; j++)
        {
            k += B[i][j] * x_last[j];
            //printf("k=%f   %f\n",B[i][j], x_last[j]);
        }
        result[i]=k+C[i];
        
    }
}

void CopyArray(float *X, float *X_last, int Size)
{
	int i;
	for (i = 0; i < Size; i++)
	{
		X_last[i] = X[i];
	}
}

void Norm(float *X, float *X1, int Size, float *Result)
{
	int i;
	for (i = 0; i < Size; i++)
	{
		Result[i] = fabsf(X[i] - X1[i]);
	}
}

float Norm_max(float *norm, int Size)
{
	int max = 0;
	for (int i = 0; i < N; ++i)
	{
		if (norm[max] < norm[i])
		{
			max = i;
		}
	}

	return norm[max];
}


int main()
{   clock_t start, stop;
    float norm[N];

    matrix();
    omp_set_num_threads(treads_num);
    clock_t start1 = clock();
    #pragma omp parallel for shared(A, B, C) private(j) 
    for (int i = 0; i < N; i++)
	{   
		C[i] = b[i] / A[i][i];
        x_last[i]=C[i];
		for (int j = 0; j < N; j++)
		{
			if (i == j)
			{
				B[i][j] = 0;
			}
			else
			{
				B[i][j] = -A[i][j] / A[i][i];
			}
		}
	}


	while (norma >= esp)
	{
		printf("Число итераций %d \n", count);
		

		multiplication(x_last, result);


		Norm(result, x_last, N, norm);

		norma = Norm_max(norm, N);
		printf("Норма %f\n", norma);
		printf("\n");
		CopyArray(result, x_last, N);
		CopyArray(zeros, result, N);

		count++;
	}
       
    clock_t stop1 = clock();
    double elapsed1 = (double)(stop1 - start1) / CLOCKS_PER_SEC;
    printf("\nTime %d thread for %d matrix =: %f\n",treads_num, N, elapsed1);



    return 0;
}

//gcc -fopenmp openmp.c -o openmp && ./openmp




