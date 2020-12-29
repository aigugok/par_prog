#include <stdio.h>
#include <mpi.h>
#include <math.h>
#include <malloc.h>
#include <time.h>
#include <stdlib.h>
#define K 1400 //размерность матрицы
#define c 2.5
#define send_data_tag 2001
#define return_data_tag 2002
float A[K][K]; // = {8, 1, 2, 0.5, 2, 1, 2, 0, 0, 0, 2, 0, 6, 0, 0, 0.5, 0, 0, 22, 0, 2, 0, 0, 0, 16};
float b[K];    // = {17, 3, 7, 6, 12};
// результат - {180, 23, 76,140.5,226}

void matrix(void) //Расчёт случайной матрицы и вектора заданной размерности
{
    float sum = 0;
    srand((unsigned int)time(NULL));
#pragma omp parallel
    {
#pragma omp for
        for (int i = 0; i < K; i++)
        {
            sum = 0; //считаем сумму по столбцам
            for (int j = 0; j < K; j++)
            {
                if (i != j)
                {
                    A[i][j] = ((float)rand() / (float)(RAND_MAX)) * c - 5.0;
                    sum += fabsf(A[i][j]);
                }
            }
            A[i][i] = 1.2 * (sum + fabsf(((float)rand() / (float)(RAND_MAX)) * c));
            b[i] = ((float)rand() / (float)(RAND_MAX)) * c - 5.0;
        }
    }
}

int main(int argc, char **argv)
{
    matrix(); //задаём матрицы и вектора

    float C_full[K + 1]; //Вся матрица С
    float x_in[K];
    float x_out[K];
    MPI_Status status;
    int an_id;
    float esp = 0.0001;//точность
    int count = 0;
    int root_process = 0;
    int ierr = 1;
    int my_id, num_procs;
    int i;
    int dj = 1;
    int iter_count = 0;
    int out_count;//количество строк, которые считает процесс +1 элемент нормы
    int i_null;//первая строка, которую обрабатывает каждый процесс
    float max_norm;//норма
    float max_norm_last;//проверочная норма
    clock_t start1 = clock();//засекает начало расчёта
    ierr = MPI_Init(&argc, &argv);//инициалиризуем процессы
    ierr = MPI_Comm_rank(MPI_COMM_WORLD, &my_id);//получаем id процесса
    ierr = MPI_Comm_size(MPI_COMM_WORLD, &num_procs);//получаем количество процессов

    // часть рутового процесса
    if (my_id == root_process)
    {
        float C[K + 1];        //матрица С, которая получается из выходных данных каждого процесса
        float x_out_pr[K + 1]; //выходные данные 0й элемент - норма каждого процесса

        for (an_id = 1; an_id < num_procs; an_id++)
        { //цикл для получения С
            if (an_id <= K % (num_procs - 1))
            {
                out_count = K / (num_procs - 1) + 2;
                i_null = (an_id - 1) * (out_count - 1);
            }
            else
            {
                out_count = K / (num_procs - 1) + 1;
                i_null = (an_id - 1) * (out_count - 1) + K % (num_procs - 1);
            }
            ierr = MPI_Recv(C, out_count - 1, MPI_FLOAT, an_id, return_data_tag, MPI_COMM_WORLD, &status);

            for (i = 0; i < out_count - 1; i++)
            {
                C_full[i_null + i] = C[i];
            }
        }

        for (i = 0; i < K; i++)
        {
            x_in[i] = C_full[i]; //задаём X1
        }
        while (dj == 1)
        {
            iter_count++; //номер итарации
            for (an_id = 1; an_id < num_procs; an_id++)
                ierr = MPI_Send(x_in, K, MPI_FLOAT,an_id, send_data_tag, MPI_COMM_WORLD); //отправляем Xk-1 всем процессам
            for (an_id = 1; an_id < num_procs; an_id++)
            {
                if (an_id <= K % (num_procs - 1))
                {
                    out_count = K / (num_procs - 1) + 2;
                    i_null = (an_id - 1) * (out_count - 1);
                }
                else
                {
                    out_count = K / (num_procs - 1) + 1; //задаём количество элементов, которые вернут процессы, в зависимости от их номера и отношения размерности матрицы к количеству процессов
                    i_null = (an_id - 1) * (out_count - 1) + K % (num_procs - 1);
                }

                ierr = MPI_Recv(x_out_pr, out_count, MPI_FLOAT,an_id, return_data_tag, MPI_COMM_WORLD, &status); //получаем данные из процесса
                for (int j = 1; j < out_count; j++)
                {
                    x_in[i_null + j - 1] = x_out_pr[j]; //считаем на основе их Xi+1
                }
                if (an_id == 1)
                    max_norm = x_out_pr[0]; //рассчитываем норму на первом процессе
                else
                {
                    if (x_out_pr[0] > max_norm)
                        max_norm = x_out_pr[0]; //расчитываем норму на последующих процессах
                }
            }
            if (max_norm < esp || iter_count > 100) //проверяем критерий остановки ограничим количество итараций, так как матрица задаётся с хорошей сходимостью
                dj = 0;                             //переменная которая определяет указанные выше критерии остановки
            if (dj == 0)
            { //если критерий остановки, то считаем время и выводим полученные данные
                clock_t stop1 = clock();
                double elapsed1 = (double)(stop1 - start1) / CLOCKS_PER_SEC;
                printf("количество итераций %d\n", iter_count); //колво итарций
                printf("норма %f\n", max_norm);                 //норма
                printf("Time for %d matrix =: %fc\n", K,elapsed1); //затраченное время
            }
            for (an_id = 1; an_id < num_procs; an_id++)
                ierr = MPI_Send(&dj, 1, MPI_INT,an_id, send_data_tag, MPI_COMM_WORLD); //если не достигнут, то продолжаем
        }
        if (dj == 0)
            MPI_Finalize(); //завершаем головной процесс
    }
    //Вычисления подпроцессов
    else
    {
        float norm_proc = 0;
        if (my_id <= K % (num_procs - 1))
        {
            out_count = K / (num_procs - 1) + 2; //считаем сколько элементов считает процесс
            i_null = (my_id - 1) * (out_count - 1);
        }
        else
        {
            out_count = K / (num_procs - 1) + 1;
            i_null = (my_id - 1) * (out_count - 1) + K % (num_procs - 1);
        }
        float x_out_pr[out_count];
        float B[out_count - 1][K];
        float C[out_count - 1];
        for (int i = 0; i < out_count - 1; i++)
        {
            for (int j = 0; j < K; j++)
            {
                if ((i_null + i) != j)
                {
                    B[i][j] = -A[i_null + i][j] / A[i_null + i][i_null + i]; //расчет нужного процессу количества строк B
                }
                else
                    B[i][j] = 0;
            }
        }

        for (int i = 0; i < out_count - 1; i++)
        {
            C[i] = b[i_null + i] / A[i_null + i][i_null + i]; //расчёт нужного процессу количеств элементов С
           
        }
        ierr = MPI_Send(C, out_count - 1, MPI_FLOAT,
                        root_process, return_data_tag, MPI_COMM_WORLD); //отправляем С
        while (dj == 1)
        {
            ierr = MPI_Recv(x_in, K, MPI_FLOAT,
                            root_process, send_data_tag, MPI_COMM_WORLD, &status); //получаем Xk для каждого процесса
            for (i = 1; i < out_count; i++)
            {
                x_out_pr[i] = C[i - 1]; //изначально равно соответсвующему С
                for (int j = 0; j < K; j++)
                {
                    x_out_pr[i] += B[i - 1][j] * x_in[j]; //метод якоби Xi=BX(i-1)+C, C присвоили ранее
                }

                norm_proc = x_out_pr[i] - x_in[i_null + i - 1]; //считаем разницу с прошлым элементом

                if (i == 1)
                    x_out_pr[0] = fabsf(norm_proc); //записываем разницу на первой итерации как норму процесса
                else
                {
                    if (norm_proc > x_out_pr[0])
                        x_out_pr[0] = fabsf(norm_proc); //если разница на любом другом процессе больше, то его считаем нормой
                }
            }
            ierr = MPI_Send(x_out_pr, out_count, MPI_FLOAT,
                            root_process, return_data_tag, MPI_COMM_WORLD); //отправляем Xk+1 root процессу
            ierr = MPI_Recv(&dj, 1, MPI_INT,
                            root_process, send_data_tag, MPI_COMM_WORLD, &status); //получаем Xk для каждого процесса
        }
        if (dj == 0)
            MPI_Finalize(); //завершаем процесс, если получили завершение
    }

    /* code */
    return 0;
    ierr = MPI_Finalize(); //завершаем все процессы
}


// mpicc -o MPI.bin ./MPI_jacobi.c && mpirun -np 8 ./MPI.bin 
