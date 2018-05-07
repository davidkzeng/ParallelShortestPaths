import random
from random import sample

import sys

writeFile = open("random.gph", "w")

randomCap = 100000
n = 200000
edges = 0

for i in range(0,n):
  randList = random.sample(range(0,n),20)
  for randI in randList:
    if (randI != i):
      writeFile.write("%d %d\n" % (i,randI))
      edges = edges + 1

writeFile.close()
print edges
