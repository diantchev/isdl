#!/bin/bash

# Build functions 
#

# Returns only the name of the file without the extension or the begining of the file path
#
function name {
	last=${1##*/}
	echo ${last%%.*}
}

# Complies the input file to produce the output file
function compile {
	echo "g++ $GCC_FLAGS $INCLUDE -fPIC -c $1 -o $2"
	g++ $GCC_FLAGS $INCLUDE -fPIC -c $1 -o $2
}

# Complies the input file to produce the output file
function assembly {
	g++ $INCLUDE -fPIC -S $1 -o $2
}


#Finds include dependencies for the specified source file
function get_include_dep {
	DEP="";
	echo "Checking dependencies for $1";
	for dep in `g++ $INCLUDE -M $1`; do
		if [ -e $dep ]; then
			DEP="${DEP} $dep"
		fi
	done;
	echo $DEP;
}

# Comiles all the source files in the specified directory
function compile_all {
	for i in `ls $1/*.cpp`; do
		echo "Validating file $i for compilation"
		objfile=$2/`name $i`.o;
		asmfile=$2/`name $i`.asm;
		should_compile="false";
		if [ ! -f $objfile ]; then
			should_compile = "true"	#Compile if the object file doesn't exist
		fi
		# Check if include files are updated
		if [ ! $should_compile = "true" ]; then
			if [ $i -nt $objfile ]; then 
				should_complie="true" 	#Compile if the source file is newer than the object file
			fi
		fi;
		if [ ! $should_compile = "true" ]; then 
			for dep in `get_include_dep $i`; do 
				if [ $dep -nt $objfile ]; then
					should_compile="true";
					break;
				fi; 
			done;
		fi;
		if [ $should_compile = "true" ]; then
			echo "Compiling $i";
			compile $i $objfile
		else
			echo "File $i hasn't change skipping compilation"
		fi
	done;
}

#Creates a shared library from the object file in the specified directory
function make {
	objects=""
	build="false"
	for i in `ls $1/*.o`; do
		objects="$objects $i";
		if [ -f $2 ]; then
			if [ $i -nt $2 ]; then
				build="true";
			fi
		else
			build="true";
		fi
	done;
}


function make_slib {
	make $@
	if [ $build = "true" ]; then
		g++ -shared $objects -o $2;
	fi
}


function make_bin {
	make $@
	if [ $build = "true" ]; then
		g++ -L$LIBPATH $objects $LIBS -o $2;
	fi
}

function clean {
	rm $1/*.o
}




