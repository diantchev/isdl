#!/bin/bash
source scripts/functions.sh


if [ ! -d obj/core/main ]; then
	echo "Crating object directory"
	mkdir -p obj/core/main
fi

if [ ! -d lib ]; then
	echo "Crating lib directory"
	mkdir -p lib
fi


echo "Building unit tests"

if [ ! -d obj/core/test ]; then
	echo "Crating object directory for tests"
	mkdir -p obj/core/test
fi

if [ ! -d bin ]; then
	echo "Crating bin directory"
	mkdir -p bin
fi

export INCLUDE="-Icore/include -Iunittest/include"

echo "Compiling main objects"

compile_all core/main obj/core/main

echo "Creating core lib"

make_slib obj/core/main lib/libcore.so

compile_all core/test obj/core/test

LIBPATH=$PWD/lib

LIBS="-lunittest -lpthread -lcore"

make_bin obj/core/test bin/coretest

LD_LIBRARY_PATH=$PWD/lib ./bin/coretest
