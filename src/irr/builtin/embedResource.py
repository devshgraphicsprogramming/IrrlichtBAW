# takes a single argument {filename}
# the variable name inside the generated file is always constant - resource, resource_len
# the output file is located next to the original, its name is similiar to the input file,
# but ends with .h

import sys

# for getting the file size
# import os

# for regex to truncate extensions
import re

print("Script working")

if  len(sys.argv) < 2 :
    print("Incorrect argument count")
else:

    #arguments
    inputfilename = sys.argv[1]
    varname = sys.argv[2]
    outputfilename = sys.argv[3]
    #outputfilename = re.sub(r'\..+', '', inputfilename)+ '.h'

    #file size
    # fsize = os.stat(inputfilename).st_size

    #opening a file
    outp = open(outputfilename,"w+")

    print("Created file at %s" % outputfilename)
    outp.write("#include <stdlib.h>\n")

    # outp.write("const uint8_t* resource[%d] = {\n" % fsize) # array size is not required
    outp.write("namespace irr { \n\tnamespace builtin { \n\t\tconst uint8_t* %s[] = {\n" % varname)

    with open(inputfilename, "rb") as f:
        index = 0
        byte = f.read(1)
        while byte != b"":
            outp.write("0x%s, " % byte.hex())
            index += 1
            if index % 10 == 0 :
                outp.write("\n")
            byte = f.read(1)
        # end of file byte
        outp.write("0x0")

        # if fsize != index :
        # print("Error, incorrect size of file and records written onto an array")
    
    outp.write( "\t\t};\n")
    outp.write("\t\tconst size_t %s_len = sizeof(%s);\n\t}\n}" % varname,varname)
    outp.close()
