#!/bin/bash

# --- إعدادات الملفات ---
SOURCE_FILE="Serial1.c"
EXEC_FILE="serial.exe"
DATASET="unemployment_200mb.csv" # <--- التعديل الأول هنا

# --- 1. التجميع (Compile) ---
echo "--- Compiling Serial Code ---"
gcc "$SOURCE_FILE" -o "$EXEC_FILE" -fopenmp

if [ $? -ne 0 ]; then
    echo "Error: Compilation failed!"
    exit 1
fi
echo "Compilation successful. Executable created: $EXEC_FILE"

echo "--- Starting Serial Benchmarks (10 runs) ---"

total_time=0

for i in {1..10}; do
    # تشغيل البرنامج وتمرير اسم الملف له <--- التعديل الثاني هنا
    output=$(./$EXEC_FILE "$DATASET") 
    
    # استخراج الوقت:
    run_time=$(echo "$output" | grep "Execution Time:" | awk '{print $3}')
    
    # التحقق من أننا حصلنا على رقم
    if [[ -z "$run_time" ]]; then
        echo "   Run $i: Error (Could not parse time - Did the program crash?)"
        continue
    fi

    echo "   Run $i: $run_time sec"
    
    # جمع الأوقات
    total_time=$(awk "BEGIN {print $total_time + $run_time}")
done

# --- 2. حساب المتوسط ---
avg_time=$(awk "BEGIN {print $total_time / 10}")

echo "---------------------------------------"
echo "✅ Serial Average Time: $avg_time seconds"
echo "---------------------------------------"

echo "Serial Average: $avg_time" > serial_result.txt