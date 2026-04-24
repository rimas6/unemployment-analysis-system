#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

typedef struct {
    char area_type[10]; // Urban or Rural
    double rate;
} Record;

#define MAX_RECORDS 9000000 

int main(int argc, char *argv[]) {
    // --- 1. التحقق من المدخلات (Command Line Arguments) ---
    if (argc < 2) {
        printf("Error: Please provide the dataset path.\n");
        printf("Usage: %s <csv_file_path>\n", argv[0]);
        return 1;
    }

    printf("\n--- Phase 1: Loading Data into RAM ---\n");
    
    // --- 2. فتح الملف الممرر بالاسم ---
    FILE *fp = fopen(argv[1], "r");
    if (fp == NULL) { 
        printf("Error: Could not open file '%s'.\n", argv[1]); 
        return 1; 
    }

    Record *data = (Record *)malloc(MAX_RECORDS * sizeof(Record));
    if (data == NULL) { printf("Memory Error!\n"); return 1; }

    char buffer[1024];
    long total_count = 0;

    fgets(buffer, 1024, fp); // Skip Header

    while (fgets(buffer, 1024, fp) && total_count < MAX_RECORDS) {
        char *token = strtok(buffer, ","); // ID
        
        token = strtok(NULL, ","); // Area_Type
        strcpy(data[total_count].area_type, token);
        
        token = strtok(NULL, ","); // Year
        
        token = strtok(NULL, ",\r\n"); // Rate
        if (token != NULL) {
            data[total_count].rate = atof(token);
            total_count++;
        }
    }
    
    fclose(fp);
    printf("Total Records: %ld\n\n", total_count);


    // --- Phase 2: Calculation ---
    printf("--- Phase 2: Processing ---\n");

    double urban_sum = 0.0;
    long urban_count = 0;
    double rural_sum = 0.0;
    long rural_count = 0;

    double start_time = omp_get_wtime();

    for (long i = 0; i < total_count; i++) {
        if (strcmp(data[i].area_type, "Urban") == 0) {
            urban_sum += data[i].rate;
            urban_count++;
        } 
        else if (strcmp(data[i].area_type, "Rural") == 0) {
            rural_sum += data[i].rate;
            rural_count++;
        }
    }

    double end_time = omp_get_wtime();
  
    double time_taken = end_time - start_time;

    printf("\n--- Results ---\n");
    if (urban_count > 0) printf("Urban Average: %.2f%%\n", urban_sum / urban_count);
    if (rural_count > 0) printf("Rural Average: %.2f%%\n", rural_sum / rural_count);
    
    printf("\nExecution Time: %f seconds\n", time_taken);

    free(data);
    return 0;
}