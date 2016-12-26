#!/bin/sh
gcc -shared -fpic -o libvisipc.so sigmsg.c visshm.c  -I../include
sudo cp libvisipc.so /usr/lib/
#gcc -o sendq -lmsgq -lshared sendmsg.c -I../include -L=./

