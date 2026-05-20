%%writefile mpi_openmp_program_N1.c
#include <mpi.h>
#include <omp.h>
#include <stdio.h>

int main(int argc, char** argv) {
    int provided;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_FUNNELED, &provided);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // TODO 1: Implementación para la región paralela OpenMP
    #pragma omp parallel num_threads(4)
    {
        int tid = omp_get_thread_num();
        int nthreads = omp_get_num_threads();
        printf("  Proceso MPI %d | Hilo OpenMP %d de %d\n", rank, tid, nthreads);
    }

    // TODO 2: Implementación para el proceso maestro (rank == 0)
    if (rank == 0) {
        int hilos = 4;
        printf("Total unidades: %d x %d = %d\n", size, hilos, size * hilos);
    }

    MPI_Finalize();
    return 0;
}
