#!/usr/bin/env python3
import glob
import math

w = 800
h = 480

filecount = glob.glob("./*_0*.png")
rows = math.ceil(len(filecount) / 5)

f = open("atlas.txt", "w")
count = 0
for row in range(0, rows):
    for column in range(0, 5):
        count += 1
        if count > len(filecount):
            break
        f.write("{} {} {} {}\n".format(w * column, h * row, w, h))

f.close()

f = open("config.txt", "w")
f.write("delayMS: 100\n")
f.close()
