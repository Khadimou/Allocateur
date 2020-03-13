#!/bin/sh
export DYLD_LIBRARY_PATH=$PWD
export DYLD_LIBRARY_PATH=$PWD/libmalloc.so
export LD_FORCE_FLAT_NAMESPACE=1
$@
