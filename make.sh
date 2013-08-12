gcc -fPIC `pkg-config gtk+-2.0 --cflags` -c gkbtce.c
gcc -shared -Wl -o gkbtce.so gkbtce.o
cp gkbtce.so ~/.gkrellm2/plugins/

