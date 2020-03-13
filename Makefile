GCC = gcc
ICC = icc

CFLAGS = -std=c99 -g3 -Wall

LIB_FILES = library.c ./test/src/test_malloc.c ./test/src/test_realloc.c ./test/src/test_limit.c ./test/src/test_mixed.c

all: main_i main_o lib

lib:
	$(GCC) -fPIC -shared -o libmalloc.so $(LIB_FILES) -ldl

main_i :
	$(ICC) $(CFLAGS) -pthread main.c $(LIB_FILES) -o $@

main_o :
	$(GCC) $(CFLAGS) -pthread main.c $(LIB_FILES) -o $@ -ldl -lm

clean:
	rm -rf main_i main_o libmalloc.so
	