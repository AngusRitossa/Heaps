# Compiles the files
echo Enter the file name
read filename
g++ -std=c++17 -Wl,-stack_size,0x1000000 -O2 -o $filename $filename.cpp