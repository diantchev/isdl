#!/bin/bash
## Common buidl script used by all the builds provides 
## the common functionality of building and running unit tests
source scripts/functions.sh

## Build to be performed
BUILD=$1

FUNCTION=$2 # This the function could be build, test clean



function help {
	echo "build.sh BUILD FUNCTION FUNCTION_PARAMETERS"
	echo "BUILD - root directory of the project to build"
	echo "FUNCION - build, clean or test"
	echo "BUILD parameters are - BUILDTYPE and BUILDMODE"
	echo "BUILDTYPE could be lib for library and bin for executable"
	echo "BUILDMODE debug for including debug information or release"
	echo "FUNCTION clean removes object file and build artifacts for the project"
	echo "FUNCTION test runs the unit test with no parameters runs all the unit test"
	echo " 			otherwise only the list specified"
}



if [ ! -d obj/$BUID/main ]; then
	echo "Crating object directory"
	mkdir -p obj/$BUILD/main
fi

if [ ! -d lib ]; then
	echo "Crating lib directory"
	mkdir -p lib
fi

export INCLUDE=-I$BUILD/include


echo "Building unit tests"

if [ ! -d obj/$BUILD/test ]; then
	echo "Crating object directory for tests"
	mkdir -p obj/$BUILD/test
fi

if [ ! -d bin ]; then
	echo "Crating bin directory"
	mkdir -p bin
fi

case $FUNCTION in

build )
	echo "building $BUILD"
	if [ $# -ne 4 ]; then
		echo "Invalid number of arguments expected 4"
		help
		exit
	fi 
	export INCLUDE=-I$BUILD/include
	if [ $4 = "debug" ]; then 
		export GCC_FLAGS="-g";
	fi;   
	echo "Compiling main objects"
	compile_all $BUILD/main obj/$BUILD/main
	if [ $3 = "lib" ]; then
		echo "Creating $BUILD library"
		make_slib obj/$BUILD/main lib/lib$BUILD.so
	else
		make_bin obj/$BUILD/main bin/$BUILD
	fi;
		
	;;
clean )
	clean $BUILD
	;;
test )
	export INCLUDE="-Iunittest/include $INCLUDE"
	LIBPATH="lib $LIBPATH"
	LIBS="-lunittest -lpthread ${LIBS}"
	UNITTESTBIN="${BUILD}test"
	echo "Building $UNITTESTBIN"
	compile_all $BUILD/test obj/$BUILD/test
	make_bin obj/$BUILD/test bin/$UNITTESTBIN
	shift 2
	LD_LIBRARY_PATH=$PWD/lib; bin/$UNITTESTBIN $@
	;;
* )
	echo "Invalid operation"
	help
	exit
esac








