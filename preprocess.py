import sys

source_file = sys.argv[1]
source = ['program main;\n']

with open(source_file) as f:
    line = f.readline()
    while line:
        #忽略空行
        if line != '\n':
            source.append(line)
        line = f.readline()
        
for lind in range(len(source)):
    if source[lind].__contains__('const'):
        source[lind] = source[lind].replace('=',':=')
    if source[lind].__contains__('procedure'):
        source[lind] = source[lind].replace(';','();')
    if source[lind].__contains__('call'):
        call_place = source[lind].find('call')
        left_sub = source[lind][:call_place]
        right_sub = source[lind][call_place:]
        if right_sub.find(';') != -1:
            right_sub = right_sub.replace(';', '();', 1)
        else:
            right_sub = right_sub.replace('\n', '()\n', 1)
        source[lind] = left_sub + right_sub
        
    if source[lind].__contains__('end.'):
        source[lind] = source[lind].replace('end.', 'end')
    if source[lind].__contains__('end'):
        if source[lind-1][len(source[lind-1])-2]==';':
            s = list(source[lind-1])
            s[len(source[lind-1])-2] = ''
            source[lind-1] = ''.join(s)
            # source[lind-1][len(source[lind-1])-2]=''
        source[lind] = source[lind].replace('end;','end')
    

for lind in range(len(source)-1):
    if source[lind].__contains__('end'):
            if source[lind+1].__contains__('end') or source[lind+1].__contains__('begin') :
                source[lind] = source[lind].replace(';','')
            else:
                source[lind] = source[lind].replace('end','end;')
                
level = 0

for lind in range(len(source)):
    if source[lind].__contains__('begin'):
        level += 1
    elif source[lind].__contains__('end'):
        level -= 1
    else:
        if level <= 0 and not source[lind].__contains__('procedure') and not source[lind].__contains__('program')and not source[lind].__contains__('var')  and not source[lind].__contains__('const'):
            if source[lind].__contains__('.'):
                del source[lind]
                print('1')
            else:
                source[lind-1] = source[lind-1].replace('end;', 'end')
                left = source[:lind]
                right = source[lind+1:]
                left.append('begin\n')
                left.append(source[lind])
                left.append('end\n')
                for str in right:
                    left.append(str)
                source = left
                level += 1

      
if source[len(source)-1]=='.':
    del source[len(source)-1]
    
f = open('tmp/source.prep', 'w')
for i in range(len(source)):
    f.write(source[i])
f.close()
