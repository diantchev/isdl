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
	g++ -fPIC -c $1 -o $2
}

# Comiles all the source files in the specified directory
function compile_all {
	for i in `ls $1/*.cpp`; do
		objfile=$2/`name $i`.o; 
		if [ -f $objfile ]; then
			if [ $i -nt $objfile ]; then
				compile $i $objfile
			fi
		else
			compile $i $objfile
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




