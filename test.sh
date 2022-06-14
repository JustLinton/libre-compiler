#code1
make clean
rm tmp/*
rm output/*
make
./grammar ./PL0_code/PL0_code0.in

#code2
g++ main.cpp -o plc
./plc input/code2.txt