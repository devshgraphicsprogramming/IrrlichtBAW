# takes a single argument {filename}
# the variable name inside the generated file is always constant - resource, resource_len
# the output file is located next to the original, its name is similiar to the input file,
# but ends with .h

import sys

# for getting the file size
# import os

# for regex to truncate extensions
import re

if  len(sys.argv) < 2 :
    print("Incorrect argument count")
else:

    #arguments
    inputfilename = sys.argv[1]
    outputfilename = inputfilename + '.h'
    #outputfilename = re.sub(r'\..+', '', inputfilename)+ '.h'

    #file size
    # fsize = os.stat(inputfilename).st_size

    #opening a file
    outp = open(outputfilename,"w+")

    outp.write("#include <stdlib.h>\n")

    # outp.write("const uint8_t* resource[%d] = {\n" % fsize) # array size is not required
    outp.write("const uint8_t* resource[] = {\n")

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
    
    outp.write( "};\n")
    outp.write("const size_t resource_len = sizeof(resource);\n\n")
    outp.close()
