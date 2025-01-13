#!/usr/bin/env python3

import os
import shutil
from os.path import join
from hashlib import md5

# $ tree  -d -L 1 --noreport
# .
# ├── 1walkmanOne
# ├── di-avlos
# ├── eclipse
# ├── nw-a30
# ├── nw-a40
# ├── nw-a50
# ├── nw-wm1a
# ├── seasons
# └── zx-300

# nw-30 tables are smaller and therefore fail on everything else
mvt_size = 84950
dsd_size = 13076
tct_size = 2888

rootdir = "/media/ssd/dev/nw/tunings"
hashes = {}
filelist = []
for root, dirs, files in os.walk(rootdir):
    for name in files:
        if name[-4:] == ".tbl":
            filelist.append(join(root, name))

for file in filelist:
    with open(file, "rb") as f:
        d = f.read()
        h = md5(d).hexdigest()
        if h not in hashes:
            hashes[h] = []
        hashes[h].append(file)

outdirs = {}
for k, v in hashes.items():
    last = sorted(v)[-1]
    if "ambgain" not in last and "ncgain" not in last:
        fname = os.path.split(last)[1]
        model = os.path.split(os.path.split(last)[0])[1]
        newname = os.path.splitext(fname)[0] + "_" + model + os.path.splitext(fname)[1]
        s = os.stat(last)
        if newname[:2] == "tc":
            if s.st_size != tct_size:
                continue
            newname = os.path.join("tone_control", newname)
        elif newname[:6] == "ov_dsd":
            if s.st_size != dsd_size:
                continue
            newname = os.path.join("master_volume_dsd", newname)
        elif newname[:2] == "ov":
            if s.st_size != mvt_size:
                continue
            newname = os.path.join("master_volume", newname)
        outdirs[last] = newname

os.makedirs("tone_control", exist_ok=True)
os.makedirs("master_volume_dsd", exist_ok=True)
os.makedirs("master_volume", exist_ok=True)

for k, v in outdirs.items():
    shutil.copy(k, v)
