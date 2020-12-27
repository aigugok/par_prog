import numpy as np

A_arr = np.array([[8, 1, 2, 0.5, 2], [1,2,0,0,0], [2,0,6,0,0],[0.5,0,0,22,0],[2,0,0,0,16]])
print(A_arr)
b_arr = np.array([[17],[3],[7],[6],[12]])
print(b_arr)
A_arr.dot(b_arr)



