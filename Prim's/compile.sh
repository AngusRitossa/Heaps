# Compiles the files
echo Enter the file name
read filename
g++ -std=c++14 -O2 -o $filename $filename.cpp