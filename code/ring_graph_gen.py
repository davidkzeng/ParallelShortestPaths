import random
from random import sample
from random import randint

import sys

writeFile = open("/afs/cs.cmu.edu/academic/class/15418-s18/data/vkang-data/ring_big.gph", "w")

n = 10000000
edges = 0

for i in range(0,n):
  temp = []
  for j in range(i-5, i+6):
    if (j != i):
      temp.append(j % n)
  randInt = random.randint(0,n-1)
  while (randInt == i or randInt in temp):
    randInt = random.randint(0,n-1)
  temp.append(randInt)
  for t in temp:
    writeFile.write("%d %d\n" % (i, t))
    edges = edges + 1

#  if(i - 5 < 0):
#    temp.append(random.randint(i + 6, (i - 5)%n)
#  elif(i + 6 >= n):
#    temp.append(random.randint((i + 6)%n, i-5))
#  else:



writeFile.close()
print edges
