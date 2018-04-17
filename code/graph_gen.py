f = open('data/long_tile500000.gph', 'w')

# Numbered from top to bottom, left to right
HEIGHT = 3
WIDTH = 200000

TOTAL = HEIGHT * WIDTH

edges = []

def to_index(r, c):
  return r * WIDTH + c

for i in range(TOTAL):
  r = i / WIDTH
  c = i % WIDTH

  if r > 0:
    edges.append((i, to_index(r - 1, c)))
  if c > 0:
    edges.append((i, to_index(r, c - 1)))
  if c < WIDTH - 1:
    edges.append((i, to_index(r, c + 1)))
  if r < HEIGHT - 1:
    edges.append((i, to_index(r + 1, c)))


f.write("%d %d\n" % (TOTAL, len(edges)))

for edge in edges:
  f.write("%d %d\n" % edge)

f.close()



