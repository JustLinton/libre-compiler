import os
import sys

source_file = sys.argv[1]
source_preprocessed_file = './tmp/source.prep'

print('[INFO] Preprocessing '+source_file+ '...')

os.system('python preprocess.py '+source_file)

print('[INFO] Generating inter code of '+source_file+ '...')
os.system('./grammar '+source_file)

print('[INFO] Compiling and running...')
os.system('./main '+source_preprocessed_file)

print('[INFO] Cleaning up...')
os.system('rm ./tmp/* ')