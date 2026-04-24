#!/bin/bash

# 1. Compile
echo "--- Compiling Code ---"
gcc -fopenmp parallel1.c -o parallel1.exe

if [ ! -f "parallel1.exe" ]; then
    echo "Error: Compilation failed!"
    exit 1
fi

# 2. CSV Header
output_file="openmp_1500.csv"
echo "Threads,Scheduling,Average_Execution_Time" > $output_file

echo "--- Starting Project Benchmarks (Calculating Averages) ---"

# Threads
thread_counts=(2 4 8 12 16)

# Configurations (Chunk 500)
declare -a configs=(
    "static"            
    "static,1500"       
    "dynamic,1500"      
    "guided,1500"       
)

# Loops
for t in "${thread_counts[@]}"; do
    
    echo "Testing: $t Threads..."

    for conf in "${configs[@]}"; do
        
        export OMP_NUM_THREADS=$t
        export OMP_SCHEDULE="$conf"

        total_time=0
        
        for run in {1..10}; do
            # --- التعديل هنا: أضفنا اسم الملف ---
            current_time=$(./parallel1.exe unemployment_200mb.csv | grep "Execution" | awk '{print $3}')
            
            # التحقق من أن الوقت ليس فارغاً (للتأكد أن البرنامج اشتغل)
            if [[ -z "$current_time" ]]; then current_time=0; fi
            
            total_time=$(awk "BEGIN {print $total_time + $current_time}")
        done

        # Average
        avg_time=$(awk "BEGIN {printf \"%.6f\", $total_time / 10}")

        echo "  -> Config: $conf | Avg Time: $avg_time sec"

        # CSV
       echo "$t,\"$conf\",$avg_time" >> $output_file
            
    done
done

echo "---------------------------------------"
echo "✅ Done! Averages saved to '$output_file'"