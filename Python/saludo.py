from mpi4py import  MPI

comm  = MPI.COMM_WORLD
rank = comm.Get_rank()
size = comm.Get_size()

print("Hola, mundo, desde el proceso", rank,"de",size)
