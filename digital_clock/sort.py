#!/usr/bin/env python3
import shutil
import os
import subprocess
from _collections import OrderedDict

# https://www.dylanmcd.com/images/blog/every-ipod-color/cards/iPod%20nano%207%20Reference%20Card.webp
colors = {
    "yellow": 229442333,
    "green": 229442332,
    "red_product": 229442336,
    "blue": 229442339,
    "blue_2012": 229442331,
    "pink": 229442338,
    "pink_2012": 229442334,
    "purple": 229442335,
    "gold": 229442337,
    "space_gray": 229442329,
    "silver": 229442330,
}
total = 40
step = 12

big = [204, 104]
medium = [92, 62]
small = [48, 30]
day = [92, 140]

misc = OrderedDict()
misc["dot"] = [74, 30]
misc["minus"] = [22, 34]
misc["shoe"] = [132, 132]
misc["colon"] = [74, 30]


def atlas_config(directory):
    out = ""
    f = open(os.path.join(directory, "atlas.txt"), "w")
    for row in range(0, 5):
        for column in range(0, 2):
            out += "{} {} {} {}\n".format(big[0] * column, big[1] * row, big[0], big[1])

    startY = big[1] * 5

    # out += "day\n"

    for row in range(0, 2):
        for column in range(0, 5):
            if row == 1 and column > 1:
                break
            out += "{} {} {} {}\n".format(
                day[0] * column, day[1] * row + startY, day[0], day[1]
            )

    # out += "med\n"

    startY += day[1] * 2

    for row in range(0, 2):
        for column in range(0, 5):
            out += "{} {} {} {}\n".format(
                medium[0] * column, medium[1] * row + startY, medium[0], medium[1]
            )

    # out += "misc\n"
    startY += medium[1] * 2

    startX = 0
    out += "{} {} {} {}\n".format(startX, startY, misc["dot"][0], misc["dot"][1])
    startX += misc["dot"][0]
    out += "{} {} {} {}\n".format(startX, startY, misc["minus"][0], misc["minus"][1])
    startX += misc["minus"][0]
    out += "{} {} {} {}\n".format(startX, startY, misc["shoe"][0], misc["shoe"][1])
    startX += misc["shoe"][0]
    out += "{} {} {} {}\n".format(startX, startY, misc["colon"][0], misc["colon"][1])
    startX += misc["colon"][0]

    # out += "small\n"

    startY += misc["shoe"][1]

    for row in range(0, 2):
        for column in range(0, 5):
            out += "{} {} {} {}\n".format(
                small[0] * column, small[1] * row + startY, small[0], small[1]
            )

    f.write(out)
    f.close()


def run():
    i = 0
    for directory, start in colors.items():
        try:
            shutil.rmtree(directory)
        except Exception as e:
            pass
        os.mkdir(directory)
        while i <= total:
            fname = "./ipod_theme/body/{}_0064.png".format(start + step * i)
            if i < 10:
                newName = "{}_big.png".format(i)
            if i >= 10 and i < 20:
                newName = "{}_small.png".format(i - 10)
            if i >= 20 and i < 30:
                newName = "{}_medium.png".format(i - 20)
            if i == 30:
                newName = "dot.png"
            if i == 31:
                newName = "colon.png"
            if i == 32:
                newName = "shoe.png"
            if i == 33:
                newName = "minus.png"
            if i > 33:
                newName = "day_{}.png".format(i - 34)  # starts with sunday

            newName = os.path.join(directory, newName)
            subprocess.run(
                [
                    "convert",
                    fname,
                    "-filter",
                    "point",
                    "-resize",
                    "200%",
                    "-background",
                    "black",
                    "-alpha",
                    "remove",
                    "-alpha",
                    "off",
                    "-rotate",
                    "-90",
                    "PNG32:" + newName,
                ]
            )
            i += 1
        i = 0

        atlas_config(directory)


run()
