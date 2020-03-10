import sys

if  len(sys.argv) < 4 :
    print("Incorrect argument count. Proper use is python -d 'exe path' 'resource name' 'input file name' 'output file name'")
else:

    #arguments
    name = sys.argv[1]
    outputfilename = sys.argv[2]+ '.h'
    inputfilename = sys.argv[3]

    #opening a file
    outp = open(outputfilename,"w+")

    outp.write("#include <stdlib.h>\n")
    outp.write("const uint8_t* %s[] = {\n" % name)

    with open(inputfilename, "rb") as f:
        index = 0
        byte = f.read(1)
        while byte != b"":
            outp.write("0x%s, " % byte.hex())
            index += 1
            if index % 10 == 0 :
                outp.write("\n")
            byte = f.read(1)
    outp.write( "};\n")
    outp.write(str.format("const size_t {0}_len = sizeof({0});\n\n", name))
    outp.close()
