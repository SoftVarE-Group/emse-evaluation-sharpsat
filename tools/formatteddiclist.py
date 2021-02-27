import os
import sys
import re
from glob import glob
from functools import cmp_to_key

# TODO: Flexible compare function, flexible expression for the safe format

sourceDicPath = sys.argv[1] # source directory as first parameter
outputPath = sys.argv[2] # outputfile as second parameter (create if not exists)


def compare(x1, x2):
    x1Number = int(re.search("[0-9]*F", x1).group()[:-1]) # find XXXXF remove F
    x2Number = int(re.search("[0-9]*F", x2).group()[:-1])
    if (x1Number >= x2Number):
        return 1
    elif (x1Number < x2Number):
        return -1
    else:
        return 0

files = []
start_dir = os.getcwd()
pattern   = "*.dimacs"

for dir,_,_ in os.walk(start_dir):
    files.extend(glob(os.path.join(dir,pattern))) 

fileList = sorted(files)
#fileList = sorted(fileList, key=cmp_to_key(compare))

f = open(outputPath, "w")
for file in fileList:
    f.write("{\"file\": \""+  os.path.join(sourceDicPath, file) + "\"},\n")

f.close()
