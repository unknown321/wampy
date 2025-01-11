#!/usr/bin/env python3

w = 800
h = 480

f = open("atlas.txt", "w")
for row in range(0,8):
    for column in range(0,5):
        f.write("{} {} {} {}\n".format( w*column, h*row, w,h))
f.close()

f = open("config.txt", "w")
f.write("delayMS: 200\n")
f.close()
