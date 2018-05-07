from random import randint
import numpy as np
import sys

datafile = sys.argv[1]
outputfile = datafile[:-4] + '_w_geom.gph'

readFile = open(datafile, "r")
writeFile = open(outputfile, "w")

randomCap = 1000

for line in readFile:
  temp = line.split()
  if len(temp) == 3:
    writeFile.write("%s %s w\n" % (temp[0],temp[1]))
  if len(temp) == 2:
    # Geometric distribution
    randI = np.random.geometric(.10);
    #randI = randint(1, randomCap)
    writeFile.write("%d %d %d\n" % (int(temp[0]),int(temp[1]), randI))

writeFile.close()
readFile.close()
