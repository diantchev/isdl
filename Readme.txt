This project contains useful C++ implementations - abriviated as ucppi
Directories are organized in the following manner. On the top level different subprojects name like
core - containing core implementation like logging, xml processing, queue implementations etc
test - including implementation of simple unittest framework
messaging - implemenation of messaging 

Each of this subprojects has its own src directory wich intern has test and main subdirectory. The main subderectory contains 
the main code and test directory all unittess. 

On top level we have bin, lib, and obj directories which contain the build binaries. All the built executables are created in the bin directory, the dynamic libraries are dropped in the lib directory, obj directory is the place where the object files are placed. Object files are generated in a subdirectory corresponding to the top level project

