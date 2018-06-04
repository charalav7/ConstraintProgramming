g++ -std=c++11 -I/usr/local/include -c $1.cpp 
g++ -o $1 $1.o -lgecodeflatzinc -lgecodedriver -lgecodegist -lgecodesearch -lgecodeminimodel -lgecodeset -lgecodefloat -lgecodeint -lgecodekernel -lgecodesupport
./$1 $2 $3 $4 $5 $6
