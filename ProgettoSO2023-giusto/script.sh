#!/bin/bash

# Nome del programma da compilare ed eseguire

flag1="$1"
flag2="$2"
flag3="$3"
flag4="$4"
# Nome del file di output

gcc -o parent parent.c -lrt -pthread > 1
gcc -o controllore controllore.c -lrt -pthread > 1

# Execute parent in the background
./parent $flag1 $flag2 $flag3 $flag4 &

# Sleep for 10 seconds
sleep 10

# Execute controllore in the background
./controllore > filelog.txt &


