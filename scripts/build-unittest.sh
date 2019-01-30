#!/bin/bash
source scripts/functions.sh


if [ ! -d obj/unittest/main ]; then
	echo "Crating object directory"
	mkdir -p obj/unittest/main
fi

if [ ! -d lib ]; then
	echo "Crating lib directory"
	mkdir -p lib
fi

echo "Compiling main objects"

compile_all unittest/main obj/unittest/main

echo "Creating untites lib"

make_slib obj/unittest/main lib/libunittest.so

echo "Building unit tests"

if [ ! -d obj/unittest/test ]; then
	echo "Crating object directory for tests"
	mkdir -p obj/unittest/test
fi

if [ ! -d bin ]; then
	echo "Crating bin directory"
	mkdir -p bin
fi

compile_all unittest/test obj/unittest/test

LIBPATH=$PWD/lib

LIBS=-lunittest

make_bin obj/unittest/test bin/unittesttest

LD_LIBRARY_PATH=$PWD/lib ./bin/unittesttest
