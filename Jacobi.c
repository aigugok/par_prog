#include <stdio.h>
#include <math.h>
#include <malloc.h>
#include <time.h>
#include <stdlib.h>

#define N 5000																					   //размерность матрицы
#define c 15.0																					   //размерность
float A[N][N] ={8, 1, 2, 0.5, 2, 1, 2, 0, 0, 0, 2, 0, 6, 0, 0, 0.5, 0, 0, 22, 0, 2, 0, 0, 0, 16}; //матрицы А
float b[N] ={17, 3, 7, 6, 12};																	   //матрицы В
float result[N];																				   //ответ
float esp = 0.00001;																				   //точночть
float norma = 1;

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
{
	float k = 0;
	int j = 0;
	while (j < N)
	{
		for (int i = 0; i < N; i++)
		{
			k += B[j][i] * x_last[i];
			// printf("B[i][j]%f\n",B[j][i] );
			// printf("x[i]%f\n",x_last[i] );
		}
		result[j] = k + C[j];
		k = 0;
		j++;
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

int main()
{

	clock_t start, stop;

	matrix();

	float norm[N];
	for (int i = 0; i < N; i++)
	{
		C[i] = b[i] / A[i][i];
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
	// for (int i = 0; i < N; i++){
	// for (int j = 0; j < N; j++)
	// 		{
	// 			printf("Matrix B i=%d j=%d %f\n",i,j, B[i][j]);
	// 		}
	// 		}

	// for (int i = 0; i < N; i++){
	// 	printf("Matrix C i=%d %f\n",i, C[i]);
	// }

	printf("Критерий остановки= %f\n", esp);

	float zeros[N] = {0};
	int count = 1;
	//while (count < 2)
	while (norma >= esp)
	{
		printf("Число итераций %d \n", count);
		if (count == 1)
		{
			CopyArray(C, x_last, N);
		}
		start = clock();
		multiplication(x_last, result);
		stop = clock();

		Norm(result, x_last, N, norm);

		norma = Norm_max(norm, N);
		printf("Норма %f\n", norma);
		printf("\n");
		CopyArray(result, x_last, N);
		CopyArray(zeros, result, N);

		count++;
	}
	// for (int i = 0; i < N; i++){
	// 	printf("Result i=%d %f\n",i, x_last[i]);
	// }

	printf("Time = %fsec.\n", (double)(stop - start) / CLOCKS_PER_SEC);
}

//gcc Jacobi.c -o jacobi && ./jacobi

