#!/usr/bin/env python3
import shutil
import os
import subprocess

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


def run():
    i = 0
    for directory, start in colors.items():
        try:
            shutil.rmtree(directory)
        except Exception as e:
            pass
        os.mkdir(directory)
        while i <= total:
            fname = "../ipod_theme/body/{}_0064.png".format(start + step * i)
            if i < 10:
                newname = "{}_big.jpg".format(i)
            if i >= 10 and i < 20:
                newname = "{}_small.jpg".format(i - 10)
            if i >= 20 and i < 30:
                newname = "{}_medium.jpg".format(i - 20)
            if i == 30:
                newname = "dot.jpg"
            if i == 31:
                newname = "colon.jpg"
            if i == 32:
                newname = "shoe.jpg"
            if i == 33:
                newname = "minus.jpg"
            if i > 33:
                newname = "day_{}.jpg".format(i - 34)  # starts with sunday

            newname = os.path.join(directory, newname)
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
                    newname,
                ]
            )
            i += 1
        i = 0


run()
