import os
import sys

source_file = sys.argv[1]
source_preprocessed_file = './tmp/source.prep'
prelex_file = './tmp/pre.lex'

print('[INFO] Preprocessing '+source_file+ '...')

os.system('python preprocess.py '+source_file)

print('[INFO] Generating inter code of '+source_file+ '...')
os.system('./grammar '+source_file)

source = ['SYM\tNAME\n','==========\n']
with open(prelex_file) as f:
    line = f.readline()
    for i in range(6):
        line = f.readline()
    while line:
        #忽略空行
        if line != '\n':
            source.append(line.split()[0]+'\t' + line.split()[1]+'\n')
        line = f.readline()
f = open('./output/lex.txt', 'w')
for i in range(len(source)):
    f.write(source[i])
f.close()

print('[INFO] Compiling and running...')
os.system('./main '+source_preprocessed_file)

print('[INFO] Cleaning up...')
os.system('rm ./tmp/* ')