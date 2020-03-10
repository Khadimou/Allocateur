GCC = gcc
ICC = icc

CFLAGS = -std=c99 -g3 -Wall

LIB_FILES = library.c

all: main_i main_o lib 

lib:
	$(GCC) -fPIC -shared -o libcustom.so $(LIB_FILES) -ldl

main_i :
	$(ICC) $(CFLAGS) -pthread main.c $(LIB_FILES) -o $@

main_o :
	$(GCC) $(CFLAGS) -Wpadded -pthread main.c $(LIB_FILES) -o $@ -ldl -lm

run :
	export LD_PRELOAD=libcustom.so 

clean:
	rm -rf main_i main_o libcustom.so
	