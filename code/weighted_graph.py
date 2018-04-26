from random import randint

readFile = open('data/g_22_22.gph', "r")
writeFile = open('data/g_22_22_w.gph', "w")

randomCap = 100

for line in readFile:
  temp = line.split()
  if len(temp) == 3:
    writeFile.write("%s %s w\n" % (temp[0],temp[1]))
  if len(temp) == 2:
    randI = randint(1, randomCap)
    writeFile.write("%d %d %d\n" % (int(temp[0]),int(temp[1]), randI))

writeFile.close()
readFile.close()
