#!/bin/bash

# Initialize the CSV file with column headers
echo "Threads, Time" > posix_results.csv

# Loop through thread counts in powers of two
for t in 1 2 4 8 16 32 64 128; do
    echo "Running POSIX with $t threads..."
    ./posixnoblocking $t >> posix_results.csv
done

echo "Data collection complete! Check posix_results.csv"
