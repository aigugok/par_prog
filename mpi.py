from mpi4py import MPI
from random import randint
import numpy as np
import time

comm = MPI.COMM_WORLD #коммутатор с общим числом процессов
size = comm.Get_size()#общее кол-во процессов
rank = comm.Get_rank()#значение процесса
N=100 #размерность матрицы
A=[]
b=[]
result=[]
esp=0.001#критерий остановки
norm=1
count=0
count_else=0
C=[]
B=[]
x_last=[]
x=[]
flag=0

def in_A_and_b_to_B_and_C():#приводим СЛАУ к виду х=Вх+С
    global C,B,x_last
    C=[0 for i in range(N)]
    x_last=[0 for i in range(N)]
    B=[[0 for i in range(N)] for j in range(N)]
    for i in range(N):
        C[i]=b[i]/A[i][i]
        x_last[i]=C[i]
        for j in range(N):
            if(i==j):
                B[i][j]=0
            else:
                B[i][j]=-A[i][j]/A[i][i]


def generate_matrix():#генерируем матрицы А и b
    global A, b
    A=[[randint(0, 15)-5 for i in range(N) ]for j in range(N)]
    b=[randint(0, 15)-5 for j in range(N)]
    for i in range (N):
        sum=0
        for j in range(N):
            if i!=j:
                sum+=abs(A[i][j])
        A[i][i]=sum+randint(0, 15)
    # print("A=",A)
    # print("b=",b)
    in_A_and_b_to_B_and_C()
    # print()
    # print("C=", C)
    # print("B=", B)


def multiply_matrix(matrix_A, matrix_b, matrix_C):#умнoжение матриц
    res=[0 for i in range(len(matrix_A))]
    for i in range(len(matrix_A)):
        k=0
        for j in range(N):
            k+=matrix_A[i][j]*matrix_b[j]
        res[i]=k+matrix_C[i]
    print(res)
    return res


def response_matrix():#сбор матрицы из строк в одну конечную
    global result
    for i in range(1,size):
        resp = comm.recv(source=i, tag=i)
        result = result + resp
    #print("result",result)
    return result



def sending_data(x_matrix):#отправка необоходимых строк матрицы В и С для кажого процесса
    int_num_iterations=N//(size-1)
    residue=N%(size-1)
    sum=0
    row=[]
    C_row=[]
    for i in range(1,size):
        if(residue>0):
            k=int_num_iterations+1
            # print(i,"Cчитай строки матрицы B= ",k)
            # print("Co строки",sum)
            residue-=1
        else:
            k=int_num_iterations
            # print(i,"Cчитай строки матрицы B= ",k)
            # print("Co строки",sum)
        for j in range(sum,sum+k):
            row.append(B[j])
            C_row.append(C[j])
        comm.send(row, dest=i, tag=1)#каждый поток получает строки матрицы B
        comm.send(x_matrix, dest=i, tag=2)#каждый поток получает матрицу x_matrix
        comm.send(C_row, dest=i, tag=3)#каждый поток получает строки матрицы C
        C_row=[]
        row=[]
        sum+=k
    

while (norm>=esp):
    if (rank==0):
        if (count==0):
            generate_matrix()
            start = time.time() #отсчитываем время
        if (flag==0):
            sending_data(x_last)
            flag=1
        if (flag==1):
            x=np.array(response_matrix())
            flag=0
            count+=1
            result=[]
            norm=np.linalg.norm(x-x_last,ord = np.inf)
            print("norm is", norm)
            x_last=x
            print("count",count)


    else:
        A_for_proc = comm.recv(source=0, tag=1)
        b_for_proc = comm.recv(source=0, tag=2)
        C_for_proc = comm.recv(source=0, tag=3)

        res=multiply_matrix(A_for_proc, b_for_proc, C_for_proc)
        comm.send(res, dest=0, tag=rank)

if(norm<esp):
    stop= time.time()
    print('Time=', float(stop - start))
    
    

#python3.8 mpi.py 
# mpiexec -n 8 python3.8 mpi.py 