#!/usr/bin/env python3
import sys

outdir = sys.argv[1]

print("config for {}".format(outdir))

w = 528
h = 116
row = 0
column = 0
f = open("{}/atlas.txt".format(outdir), "w")
for i in range(1, 58):
    column = (i - 1) % 4
    f.write("{} {} {} {}\n".format(column * w, row * h, w, h))
    if i % 4 == 0:
        row += 1

f.close()
