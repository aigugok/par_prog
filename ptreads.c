#include <stdio.h>
#include <math.h>
#include <malloc.h>
#include <time.h>
#include <pthread.h>
#include <stdlib.h>

#define N 15000
#define c 15.0
float A[N][N]; // = {8, 1, 2, 0.5, 2, 1, 2, 0, 0, 0, 2, 0, 6, 0, 0, 0.5, 0, 0, 22, 0, 2, 0, 0, 0, 16};
float b[N];	   // = {17, 3, 7, 6, 12};
float C[N];	   //= {2.125 , 1.5, 1.16666667, 0.27272727, 0.75};
float result[N];
float esp = 0.00001; //точноcть
float norma = 1;

int threads_number = 8;
int count, residue, int_num_iterations = 0;

// структура для данных потока
typedef struct
{
	float *x_array;
	float *x_out;
	int from;
	int to;
	int row;
} threadData;

//генерируем матрицу
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

void *threadJac(void *threadJacData)
{
	threadData *data = (threadData *)threadJacData;
	float k;

	for (int i = data->from; i < data->to; i++)
	{
		// printf("threadData1[i].from %d\n", data->from);
		// printf("threadData1[i].to %d\n",data->to);
		k = 0;
		for (int j = 0; j < N; j++)
		{
			k += A[i][j] * data->x_array[j];
		}
		data->x_out[i] = k; // + C[i];
		//printf("k=%f\n", k);
	}
	pthread_exit(0);
}

int main()
{
	matrix();
	clock_t start, stop;
	start = clock();

	pthread_t *threads = (pthread_t *)malloc(N * sizeof(pthread_t));
	threadData *threadData1 = (threadData *)malloc(N * sizeof(threadData));

	int_num_iterations = N / threads_number;
	residue = N % threads_number;
	for (int i = 0; i < threads_number; i++)
	{
		threadData1[i].x_array = b;
		//printf("threadData1[i].x_array %f\n", threadData1[i].x_array[i]);
		threadData1[i].row = N;
		threadData1[i].x_out = result;
		threadData1[i].from = i * int_num_iterations;
		threadData1[i].to = (i + 1) * int_num_iterations;
		if (i == threads_number - 1)
		{
			threadData1[i].to = threadData1[i].to + residue;
		}
	}

	for (int i = 0; i < threads_number; i++)
	{
		if (i == threads_number - 1)
		{
			printf("%d потоку нужно сосчитать %d\n", i, residue + int_num_iterations);
		}
		else
			printf("%d потоку нужно сосчитать %d\n", i, int_num_iterations);
	}

	//Каждый поток, кроме последнего получит число = целочисленному делению размрности матрицы N/кол-во потоков
	//Последний посчитает + к этому остаток от деления N % кол-во потоков
	for (int i = 0; i < threads_number; i++)
	{

		pthread_create(&(threads[i]), NULL, threadJac, (void *)&threadData1[i]);
	}
	//ждем пока каждый отработает
	for (int i = 0; i < threads_number; i++)
		pthread_join(threads[i], NULL);

	// for (int i = 0; i < N; i++)
	// 	{
	// 		printf("Результат%f\n", result[i]);
	// 	}

	stop = clock();
	printf("Time for matrix with %d dimension on %d threads = %fsec.\n", N, threads_number, (double)(stop - start) / CLOCKS_PER_SEC);

	free(threads);
	free(threadData1);
}

//gcc ptreads.c -lpthread -o phtreads && ./phtreads
