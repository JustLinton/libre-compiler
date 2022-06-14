import os
import sys

print('[INFO] Building pl0 compiler...')
os.system('make clean')
os.system('make')
os.system('g++ main.cpp -o main')
print('[INFO] Build finished.')