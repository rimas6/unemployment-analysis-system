Final Project Submission: Parallel Unemployment Analysis

I. Project Overview
Goal: Calculate the average unemployment rate for Urban and Rural areas from the provided large dataset (unemployment_200mb.csv).
Implementations: Serial, OpenMP (Shared Memory), and MPI (Distributed Memory).
Final Results: The code successfully demonstrated peak performance at 8 cores/processes.

II. Prerequisites & Environment
Environment: All compilation and execution were performed using GCC in a MinGW / Git Bash environment on Windows, linked to the Microsoft MPI SDK.
Data Placement: The file unemployment_200mb.csv must be placed in the root directory of the project.

III. Build Instructions (Compilation)
Note: The -fopenmp flag is required even for the Serial code because it uses OpenMP timers.

Serial (serial.exe)
gcc -fopenmp serial/Serial1.c -o serial/serial.exe

OpenMP (parallel1.exe)
gcc -fopenmp parallel_omp/parallel1.c -o parallel_omp/parallel1.exe

MPI (mpi1.exe)
This requires linking MS-MPI manually:
gcc distributed_mpi/mpi1.c -I"/c/Program Files (x86)/Microsoft SDKs/MPI/Include" "/c/Program Files (x86)/Microsoft SDKs/MPI/Lib/x64/msmpi.lib" -lws2_32 -o distributed_mpi/mpi1.exe

IV. Run Instructions & Reproducibility
All executables require passing the dataset path as the first command-line argument.

Serial (Baseline Time)
cd serial/
./ser.sh

OpenMP (Scaling Tests)
cd parallel_omp/
Example:
./1000.sh
The results are generated into files like: openmp_1000.csv

MPI (Distributed Scaling)
cd distributed_mpi/
Example manual run:
"/c/Program Files/Microsoft MPI/Bin/mpiexec.exe" -n 8 ./mpi1.exe unemployment_200mb.csv

V. Repository Structure
Group5_FinalProject/
|-- unemployment_200mb.csv
|-- README.txt
|-- serial/
| |-- Serial1.c
| |-- ser.sh
|-- parallel_omp/
| |-- parallel1.c
| |-- 500.sh
| |-- 1000.sh
| |-- 1500.sh
| |-- openmp_XXX.csv
|-- distributed_mpi/
|-- mpi1.c