#!/bin/sh
#gcc -o ccenter -lvisipc -lnvram -lshared callcenter.c -I../include -L=./
gcc -o gcc -o /usr/local/bin/ccenter -lvisipc -lvisdb ../vislib/mbusshm.o ../vislib/visstr.o ../vislib/unit.o callcenter.c -I../include -L=./
