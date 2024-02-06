#!/bin/bash

# imposto gli argomenti

flag1="$1"
flag2="$2"
flag3="$3"
flag4="$4"

#compilo i codici sorgente

gcc -o parent parent.c -lrt -pthread > 1 
gcc -o controllore controllore.c -lrt -pthread > 1


./controllore > filelog.txt 2>&1 & #eseguo il controllore in background

./parent $flag1 $flag2 $flag3 $flag4  #eseguo il parent con i flag (argomenti)




