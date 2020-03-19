# Creates a c++ file for builtin resources that contains binary data of all resources


# parameters are
#0 - path to the .py file
#1 - output file path
#2 - cmake source dir
#3 - list of paths to resource files

import sys

# for getting the file size
import os

if  len(sys.argv) < 4 :
    print(sys.argv[0] + " - Incorrect argument count")
else:

    #arguments
    outputFilename = sys.argv[1]
    cmakeSourceDir = sys.argv[2]
    resourcePaths = sys.argv[3].split(';')
    print(sys.argv[3])
    #opening a file


    outp = open(outputFilename,"w+")
  
    outp.write("#include \"irr/builtin/builtinResources.h\"\n\n")
    outp.write("using namespace irr;\n")
    outp.write("using namespace irr/builtin;\n\n")
  
    # writing binary  data of all files in a loop
    for x in resourcePaths:
        outp.write('\n\ttemplate std::pair<uint8_t*,size_t> get_resource<IRR_CORE_UNIQUE_STRING_LITERAL_TYPE("%s")>()' % x)
        outp.write('\n\t{')
        outp.write('\n\t\tstatic const uint8_t data[] = {\n')
        
        with open(cmakeSourceDir+'/'+x, "rb") as f:
            index = 0
            byte = f.read(1)
            while byte != b"":
                outp.write("0x%s, " % byte.hex())
                index += 1  
                if index % 20 == 0 :
                    outp.write("\n")
                byte = f.read(1)
        # end of file byte
        outp.write("0x0")
        outp.write('\n\t\t};')
        outp.write('\n\t\treturn {data,sizeof(data)};')
        outp.write('\n\t}\n\n')

    outp.close()