#! /usr/bin/python

### Simple message compiler script it takes the input message file and generate an output header file with the messages
###

import sys
import re
import string


def define_class ( class_name, message ) :
	# Iterate through 
	template = "\ntemplate < "
	function = " inline std::string {0}".format(class_name)+" ( "
	function_body = " { \n\tstd::ostringstream out; \n\tout " 
	format = string.Formatter()
	max=-1
	for i in format.parse(message):
		stripped = i[0].replace("\"","")
		function_body += "<< \"{0}\" ".format ( stripped )	
		if i[1] != None: 
			function_body += "<< P{0} ".format(i[1])
			if int(i[1]) > max:
				max = int(i[1])

	function_body += ";\n\treturn out.str(); \n }"

	for i in range ( 0, max+1 ):
		template += "typename T{0}, ".format(i)
		function += "T{0}  P{0}, ".format(i) 

	if function[-2:]==", ":
		function = function[:-2]
	function += " ) "

	if len(template) > 3:
		template=template[:-2]
	template += " > \n "

	if max == -1:
		template = "\n "
		

	outfile.write ( template+function )
	outfile.write ( function_body )



if len(sys.argv) < 2:
	print ("Invalid number of arguments {0}".format(len(sys.argv)))
	print ("messagec.py --msgfile <message file> --hppfile <generated hpp file>")
	sys.exit()

try:
	msgfile = sys.argv[sys.argv.index("--msgfile")+1]
except ValueError:
	print ("Input message file is not specified ")
	sys.exit()

try:
	hppfile = sys.argv[sys.argv.index("--hppfile")+1]
except ValueError:
	print ("Output file parameter is not specified")
	sys.exit()

try:
	sys.argv[sys.argv.index("--help")+1]
	print ("messagec.py --msgfile <message file> --hppfile <generated hpp file>")
	sys.exit()
except ValueError:
	pass


print ("Generating output file {0} for message file {1}".format(hppfile, msgfile))

##Open and read the input file and generate the corresponding output file

infile = open ( msgfile )

outfile = open ( hppfile, "w" )


#outfile.write ( "#ifndef __{0}\n".format(hppfile.replace(".","_")))
#outfile.write ( "#define __{0}\n".format(hppfile.replace(".","_")))
outfile.write ( "#pragma once\n")
outfile.write ( "#include <string>\n")
outfile.write ( "#include <sstream>\n")


namespace = 0

for currLine in infile:
	#Break the line in two words
	if len( currLine.strip() ) :
		if currLine.strip()[0] == "#":
			continue
	words = re.findall( "\".+\"|\S+", currLine )
	if len(words)>0 : 
		if words[0]=="package":
			#define the namespace
			if namespace :
				outfile.write( "}\n" );
			outfile.write ( "namespace {0} ".format(words[1])+" { \n" )
			namespace = 1
		else:
			if len(words) == 2:
				define_class ( words[0], words[1] )


if namespace :
	outfile.write ( "\n}" )

#outfile.write ("\n#endif")

infile.close()
outfile.close()
	

## See if we have any namespece defined








