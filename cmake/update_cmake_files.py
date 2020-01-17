# update cpp and hpp file-lists in CMakeLists.txt files

import glob


def updateFileLists(directory, lists):
    # read CMakeLists.txt
    inputFile = open(directory + "/CMakeLists.txt", "r")
    text = inputFile.read()
    inputFile.close()
    # update lists
    for list in lists:
        filelist = glob.glob(directory + "/**/" + list[1], recursive=True)
        filelist.sort()
        filelistStr = ""
        for l in filelist:
            isException = False
            for e in list[2]:
                isException |= l.find("/" + e) != -1
            if not isException:
                filelistStr += "\n    "
                filelistStr += l[len(directory) + 1 :]
        index = text.find("set(" + list[0])
        if index == -1:
            print("Error: set(" + list[0] + "... in " + directory + "/CMakeLists.txt not found")
            return
        indexEnd = text.find(")", index)
        if index == -1:
            print("Error: set" + list[0] + "... in " + directory + "/CMakeLists.txt: no closing bracket found")
        text = text[:index] + "set(" + list[0] + filelistStr + "\n" + text[indexEnd:]
    # write CMakeLists.txt
    outputFile = open(directory + "/CMakeLists.txt", "w")
    outputFile.write(text)
    outputFile.close()


updateFileLists("ql", [("QuantLib_SRC", "*.cpp", []), ("QuantLib_HDR", "*.hpp", ["config.hpp"])])
updateFileLists("test-suite", [("QuantLib-Test_SRC", "*.cpp", ["quantlibbenchmark.cpp"])])
