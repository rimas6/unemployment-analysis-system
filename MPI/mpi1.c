/*
* Project: Global vs. Urban Unemployment Analysis System
*/
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

// Basic Settings
#define MAX_RECORDS 9000000
#define AREA_TYPE_LEN 10

// Data Structure
typedef struct {
    char area_type[AREA_TYPE_LEN];
    double rate;
} Record;

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Ref:https://www.open-mpi.org/doc/v4.1/man3/MPI_Type_create_struct.3.php
    // Define MPI Struct (To send custom data) ---
    const int nitems = 2;
    int blocklengths[2] = {AREA_TYPE_LEN, 1};
    MPI_Datatype types[2] = {MPI_CHAR, MPI_DOUBLE};
    MPI_Aint offsets[2];
    offsets[0] = offsetof(Record, area_type);
    offsets[1] = offsetof(Record, rate);
    MPI_Datatype mpi_record_type;
    MPI_Type_create_struct(nitems, blocklengths, offsets, types, &mpi_record_type);
    MPI_Type_commit(&mpi_record_type);
    
    Record *full_data = NULL;
    long total_count_long = 0;
    int count_per_process = 0; // Number of records per process

    if (rank == 0) {
        // --- 1. التحقق من المدخلات (Command Line Arguments) ---
        if (argc < 2) {
            printf("Error: Please provide the dataset path.\n");
            printf("Usage: mpiexec -n <p> %s <csv_file>\n", argv[0]);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        // --- 2. فتح الملف الممرر بالاسم ---
        FILE *fp = fopen(argv[1], "r");
        if (fp == NULL) {
            printf("Error: File '%s' not found.\n", argv[1]);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        // Allocate memory for the full dataset
        full_data = (Record *)malloc(MAX_RECORDS * sizeof(Record));
        char buffer[1024];

        // Skip CSV Header
        fgets(buffer, 1024, fp);

        // Read file line by line
        while (fgets(buffer, 1024, fp) && total_count_long < MAX_RECORDS) {
            char *token = strtok(buffer, ",");
            token = strtok(NULL, ",");

            // Store Area Type
            if(token) strncpy(full_data[total_count_long].area_type, token, AREA_TYPE_LEN - 1);
            full_data[total_count_long].area_type[AREA_TYPE_LEN - 1] = '\0';

            token = strtok(NULL, ",");
            token = strtok(NULL, ",\r\n");

            // Store Rate
            if (token != NULL) {
                full_data[total_count_long].rate = atof(token);
                total_count_long++;
            }
        }
        fclose(fp);

        // Print total records read
        printf("Total records read from file: %ld\n", total_count_long);

        // --- Auto-Adjustment: Handle non-divisible data ---
        // بدلاً من إيقاف البرنامج، نحسب الباقي ونتجاهله
        long remainder = total_count_long % size;
        if (remainder != 0) {
            // إذا كان الرانك 0، نعطي تحذير بسيط للمستخدم
            printf("\n[Warning]: Data size (%ld) is not divisible by %d processes.\n", total_count_long, size);
            printf("           Ignoring the last %ld records to ensure equal distribution.\n", remainder);
            
            // نقلل العدد الكلي ليصبح قابلاً للقسمة
            total_count_long = total_count_long - remainder;
        }

        // Calculate chunk size for each process
        count_per_process = total_count_long / size;
    }

    // A: Broadcast the chunk size to all processes
    MPI_Bcast(&count_per_process, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // B: Allocate local memory based on received size
    Record *local_data = (Record *)malloc(count_per_process * sizeof(Record));

    // C: Scatter data evenly to all processes
    MPI_Scatter(full_data, count_per_process, mpi_record_type,
                local_data, count_per_process, mpi_record_type,
                0, MPI_COMM_WORLD);

    // Free Rank 0 memory to save space
    if (rank == 0) { free(full_data); }

    MPI_Barrier(MPI_COMM_WORLD);
    double start_total = MPI_Wtime();

    // ---- Start Computation Phase ----
    MPI_Barrier(MPI_COMM_WORLD);
    double start_comp = MPI_Wtime();

    double local_urban_sum = 0.0; long local_urban_count = 0;
    double local_rural_sum = 0.0; long local_rural_count = 0;

    // Process local data (Calculate sums)
    for (int i = 0; i < count_per_process; i++) {
        char type = local_data[i].area_type[0]; // Check first letter for speed
        if (type == 'U') { // Urban
            local_urban_sum += local_data[i].rate;
            local_urban_count++;
        }
        else if (type == 'R') { // Rural
            local_rural_sum += local_data[i].rate;
            local_rural_count++;
        }
    }
    // ---- End Computation Phase ----
    double end_comp = MPI_Wtime();
    MPI_Barrier(MPI_COMM_WORLD);
    double t_comp = (end_comp - start_comp);

    // ---- Start Communication Phase ----
    MPI_Barrier(MPI_COMM_WORLD);
    double start_comm = MPI_Wtime();

    double total_urban_sum, total_rural_sum;
    long total_urban_count, total_rural_count;

    // Aggregate results using Reduce (Summing up values)
    MPI_Reduce(&local_urban_sum, &total_urban_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&local_urban_count, &total_urban_count, 1, MPI_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&local_rural_sum, &total_rural_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&local_rural_count, &total_rural_count, 1, MPI_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

    // ---- End Communication Phase ----
    MPI_Barrier(MPI_COMM_WORLD);
    double end_comm = MPI_Wtime();
    double t_comm = (end_comm - start_comm);

    // ---- End Total Time ----
    MPI_Barrier(MPI_COMM_WORLD);
    double end_total = MPI_Wtime();
    double t_total = end_total - start_total;

    // --- 5. Display Results (Rank 0) ---
    if (rank == 0) {
        printf("\n--- Analysis Results ---\n");
        if (total_urban_count > 0) printf("Urban Average: %.2f%%\n", total_urban_sum / total_urban_count);
        if (total_rural_count > 0) printf("Rural Average: %.2f%%\n", total_rural_sum / total_rural_count);

        printf("Communication Time = %.6f\n"
               "Computation Time = %.6f\n"
               "Total Time = %.6f\n",
               t_comm, t_comp, t_total);
    }
    // Final Cleanup
    free(local_data);
    MPI_Type_free(&mpi_record_type);
    MPI_Finalize();
    return 0;
}